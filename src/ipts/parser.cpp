// SPDX-License-Identifier: GPL-2.0-or-later

#include "parser.hpp"

#include "protocol.h"

#include <common/access.hpp>
#include <common/types.hpp>

#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>

void IptsParser::read(const std::span<u8> dest)
{
	iptsd::common::access::ensure(this->index + dest.size(), this->data.size());

	auto begin = this->data.begin();
	std::advance(begin, this->index);

	auto end = begin;
	std::advance(end, dest.size());

	std::copy(begin, end, dest.begin());
	this->index += dest.size();
}

void IptsParser::skip(const size_t size)
{
	this->index += size;
}

void IptsParser::reset()
{
	this->index = 0;
	std::fill(this->data.begin(), this->data.end(), 0);
}

void IptsParser::parse()
{
	const auto header = this->read<struct ipts_data>();

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

void IptsParser::parse_payload()
{
	const auto payload = this->read<struct ipts_payload>();

	for (u32 i = 0; i < payload.frames; i++) {
		const auto frame = this->read<struct ipts_payload_frame>();

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

void IptsParser::parse_hid()
{
	const auto report = this->read<u8>();

	// Make sure that we only handle singletouch inputs.
	if (report != IPTS_SINGLETOUCH_REPORT_ID)
		return;

	const auto singletouch = this->read<struct ipts_singletouch_data>();

	IptsSingletouchData data;
	data.touch = singletouch.touch;
	data.x = singletouch.x;
	data.y = singletouch.y;

	if (this->on_singletouch)
		this->on_singletouch(data);
}

void IptsParser::parse_stylus(const struct ipts_payload_frame &frame)
{
	u32 size = 0;

	while (size < frame.size) {
		const auto report = this->read<struct ipts_report>();

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

void IptsParser::parse_stylus_report(const struct ipts_report &report)
{
	IptsStylusData stylus;

	const auto stylus_report = this->read<struct ipts_stylus_report>();
	stylus.serial = stylus_report.serial;

	for (u8 i = 0; i < stylus_report.elements; i++) {
		if (report.type == IPTS_REPORT_TYPE_STYLUS_V1) {
			const auto data = this->read<struct ipts_stylus_data_v1>();

			stylus.mode = data.mode;
			stylus.x = data.x;
			stylus.y = data.y;
			stylus.pressure = data.pressure * 4;
			stylus.azimuth = 0;
			stylus.altitude = 0;
			stylus.timestamp = 0;
		}

		if (report.type == IPTS_REPORT_TYPE_STYLUS_V2) {
			const auto data = this->read<struct ipts_stylus_data_v2>();

			stylus.mode = data.mode;
			stylus.x = data.x;
			stylus.y = data.y;
			stylus.pressure = data.pressure;
			stylus.azimuth = data.azimuth;
			stylus.altitude = data.altitude;
			stylus.timestamp = data.timestamp;
		}

		if (this->on_stylus)
			this->on_stylus(stylus);
	}
}

void IptsParser::parse_heatmap(const struct ipts_payload_frame &frame)
{
	u32 size = 0;
	bool has_hm = false;
	bool has_dim = false;
	bool has_timestamp = false;

	struct ipts_heatmap_dim dim {};
	struct ipts_heatmap_timestamp time {};

	while (size < frame.size) {
		const auto report = this->read<struct ipts_report>();

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

void IptsParser::parse_heatmap_data(const struct ipts_heatmap_dim &dim,
				    const struct ipts_heatmap_timestamp &time)
{
	if (this->heatmap) {
		if (this->heatmap->width != dim.width || this->heatmap->height != dim.height)
			this->heatmap.reset(nullptr);
	}

	if (!this->heatmap)
		this->heatmap = std::make_unique<IptsHeatmap>(dim.width, dim.height);

	this->read(std::span(this->heatmap->data));

	this->heatmap->y_min = dim.y_min;
	this->heatmap->y_max = dim.y_max;
	this->heatmap->x_min = dim.x_min;
	this->heatmap->x_max = dim.x_max;
	this->heatmap->z_min = dim.z_min;
	this->heatmap->z_max = dim.z_max;

	this->heatmap->count = time.count;
	this->heatmap->timestamp = time.timestamp;
}
