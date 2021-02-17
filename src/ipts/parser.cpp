// SPDX-License-Identifier: GPL-2.0-or-later

#include "parser.hpp"

#include "protocol.h"

#include <common/types.hpp>

#include <cstring>
#include <stdexcept>
#include <utility>

IptsHeatmap::IptsHeatmap(u8 width, u8 height) : data(width * height)
{
	this->width = width;
	this->height = height;
	this->size = width * height;
}

IptsParser::IptsParser(size_t size)
{
	this->size = size;
	this->heatmap = nullptr;
	this->data = new u8[size];

	this->on_singletouch = nullptr;
	this->on_stylus = nullptr;
	this->on_heatmap = nullptr;
}

IptsParser::~IptsParser(void)
{
	delete std::exchange(this->heatmap, nullptr);
	delete[] std::exchange(this->data, nullptr);
}

u8 *IptsParser::buffer(void)
{
	return this->data;
}

void IptsParser::read(void *dest, size_t size)
{
	if (!dest)
		throw std::invalid_argument("Destination buffer is NULL");

	if (this->current + size > this->size)
		throw std::out_of_range("Reading beyond buffer size");

	std::memcpy(dest, &this->data[this->current], size);
	this->current += size;
}

void IptsParser::skip(size_t size)
{
	if (this->current + size > this->size)
		throw std::out_of_range("Reading beyond buffer size");

	this->current += size;
}

void IptsParser::reset(void)
{
	this->current = 0;
	memset(this->data, 0, this->size);
}

void IptsParser::parse(void)
{
	auto header = this->read<struct ipts_data>();

	switch (header.type) {
	case IPTS_DATA_TYPE_PAYLOAD:
		this->parse_payload();
		break;
	case IPTS_DATA_TYPE_HID_REPORT:
		this->parse_hid();
		break;
	}

	this->reset();
}

void IptsParser::parse_payload(void)
{
	auto payload = this->read<struct ipts_payload>();

	for (u32 i = 0; i < payload.frames; i++) {
		auto frame = this->read<struct ipts_payload_frame>();

		switch (frame.type) {
		case IPTS_PAYLOAD_FRAME_TYPE_STYLUS:
			this->parse_stylus(frame);
			break;
		case IPTS_PAYLOAD_FRAME_TYPE_HEATMAP:
			this->parse_heatmap(frame);
			break;
		default:
			this->skip(frame.size);
			break;
		}
	}
}

void IptsParser::parse_hid(void)
{
	auto report = this->read<u8>();

	// Make sure that we only handle singletouch inputs.
	if (report != IPTS_SINGLETOUCH_REPORT_ID)
		return;

	auto singletouch = this->read<struct ipts_singletouch_data>();

	IptsSingletouchData data;
	data.touch = singletouch.touch;
	data.x = singletouch.x;
	data.y = singletouch.y;

	if (this->on_singletouch)
		this->on_singletouch(data);
}

void IptsParser::parse_stylus(struct ipts_payload_frame frame)
{
	u32 size = 0;

	while (size < frame.size) {
		auto report = this->read<struct ipts_report>();

		switch (report.type) {
		case IPTS_REPORT_TYPE_STYLUS_V1:
		case IPTS_REPORT_TYPE_STYLUS_V2:
			this->parse_stylus_report(report);
			break;
		default:
			this->skip(report.size);
			break;
		}

		size += report.size + sizeof(report);
	}
}

void IptsParser::parse_stylus_report(struct ipts_report report)
{
	auto stylus_report = this->read<struct ipts_stylus_report>();
	this->stylus.serial = stylus_report.serial;

	for (u8 i = 0; i < stylus_report.elements; i++) {
		if (report.type == IPTS_REPORT_TYPE_STYLUS_V1) {
			auto data = this->read<struct ipts_stylus_data_v1>();

			this->stylus.mode = data.mode;
			this->stylus.x = data.x;
			this->stylus.y = data.y;
			this->stylus.pressure = data.pressure * 4;
			this->stylus.azimuth = 0;
			this->stylus.altitude = 0;
			this->stylus.timestamp = 0;
		}

		if (report.type == IPTS_REPORT_TYPE_STYLUS_V2) {
			auto data = this->read<struct ipts_stylus_data_v2>();

			this->stylus.mode = data.mode;
			this->stylus.x = data.x;
			this->stylus.y = data.y;
			this->stylus.pressure = data.pressure;
			this->stylus.azimuth = data.azimuth;
			this->stylus.altitude = data.altitude;
			this->stylus.timestamp = data.timestamp;
		}

		if (this->on_stylus)
			this->on_stylus(this->stylus);
	}
}

void IptsParser::parse_heatmap(struct ipts_payload_frame frame)
{
	u32 size = 0;
	bool has_hm = false;
	bool has_dim = false;
	bool has_timestamp = false;

	struct ipts_heatmap_dim dim;
	struct ipts_heatmap_timestamp time;

	while (size < frame.size) {
		auto report = this->read<struct ipts_report>();

		switch (report.type) {
		case IPTS_REPORT_TYPE_HEATMAP_TIMESTAMP: {
			time = this->read<struct ipts_heatmap_timestamp>();
			has_timestamp = true;
			break;
		}
		case IPTS_REPORT_TYPE_HEATMAP_DIM: {
			dim = this->read<struct ipts_heatmap_dim>();
			has_dim = true;
			break;
		}
		case IPTS_REPORT_TYPE_HEATMAP: {
			if (!has_dim || !has_timestamp)
				break;

			this->parse_heatmap_data(dim, time);
			has_hm = true;
			break;
		}
		default:
			this->skip(report.size);
			break;
		}

		size += report.size + sizeof(struct ipts_report);
	}

	if (!has_hm)
		return;

	if (this->on_heatmap)
		this->on_heatmap(*this->heatmap);
}

void IptsParser::parse_heatmap_data(struct ipts_heatmap_dim dim, struct ipts_heatmap_timestamp time)
{
	if (this->heatmap) {
		if (this->heatmap->width != dim.width || this->heatmap->height != dim.height)
			delete std::exchange(this->heatmap, nullptr);
	}

	if (!this->heatmap)
		this->heatmap = new IptsHeatmap(dim.width, dim.height);

	this->read(this->heatmap->data.data(), this->heatmap->size);

	this->heatmap->y_min = dim.y_min;
	this->heatmap->y_max = dim.y_max;
	this->heatmap->x_min = dim.x_min;
	this->heatmap->x_max = dim.x_max;
	this->heatmap->z_min = dim.z_min;
	this->heatmap->z_max = dim.z_max;

	this->heatmap->count = time.count;
	this->heatmap->timestamp = time.timestamp;
}
