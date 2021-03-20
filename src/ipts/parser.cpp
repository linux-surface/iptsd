// SPDX-License-Identifier: GPL-2.0-or-later

#include "parser.hpp"

#include "protocol.h"

#include <common/access.hpp>
#include <common/types.hpp>

#include <bitset>
#include <cstring>
#include <gsl/gsl>
#include <memory>
#include <stdexcept>
#include <utility>

namespace iptsd::ipts {

void Parser::read(const gsl::span<u8> dest)
{
	common::ensure(this->index + dest.size(), this->data.size());

	auto begin = this->data.begin();
	std::advance(begin, this->index);

	auto end = begin;
	std::advance(end, dest.size());

	std::copy(begin, end, dest.begin());
	this->index += dest.size();
}

void Parser::skip(const size_t size)
{
	this->index += size;
}

void Parser::reset()
{
	this->index = 0;
	std::fill(this->data.begin(), this->data.end(), 0);
}

void Parser::parse(bool reset)
{
	const auto header = this->read<struct ipts_data>();

	switch (header.type) {
	case IPTS_DATA_TYPE_PAYLOAD:
		this->parse_payload();
		break;
	case IPTS_DATA_TYPE_HID_REPORT:
		this->parse_hid(header);
		break;
	default:
		this->skip(header.size);
	}

	if (reset)
		this->reset();
}

void Parser::parse_loop()
{
	while (this->index < this->data.size())
		this->parse(false);

	this->reset();
}

void Parser::parse_payload()
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

void Parser::parse_hid(const struct ipts_data &header)
{
	const auto report = this->read<u8>();

	switch (report) {
	case IPTS_HID_REPORT_SINGLETOUCH:
		this->parse_singletouch();
		break;
	case IPTS_HID_REPORT_HEATMAP:
		this->parse_hid_heatmap(header);
		break;
	default:
		this->skip(header.size - sizeof(report));
		break;
	}
}

void Parser::parse_singletouch()
{
	const auto singletouch = this->read<struct ipts_singletouch_data>();

	SingletouchData data;
	data.touch = singletouch.touch;
	data.x = singletouch.x;
	data.y = singletouch.y;

	if (this->on_singletouch)
		this->on_singletouch(data);
}

void Parser::parse_stylus(const struct ipts_payload_frame &frame)
{
	u32 size = 0;

	while (size < frame.size) {
		if (size + sizeof(struct ipts_report) > frame.size)
			break;

		const auto report = this->read<struct ipts_report>();
		size += sizeof(struct ipts_report);

		if (size + report.size > frame.size)
			break;

		switch (report.type) {
		case IPTS_REPORT_TYPE_STYLUS_V1:
		case IPTS_REPORT_TYPE_STYLUS_V2:
			this->parse_stylus_report(report);
			break;
		default:
			this->skip(report.size);
			break;
		}

		size += report.size;
	}

	this->skip(frame.size - size);
}

void Parser::parse_stylus_report(const struct ipts_report &report)
{
	StylusData stylus;

	const auto stylus_report = this->read<struct ipts_stylus_report>();
	stylus.serial = stylus_report.serial;

	for (u8 i = 0; i < stylus_report.elements; i++) {
		if (report.type == IPTS_REPORT_TYPE_STYLUS_V1) {
			const auto data = this->read<struct ipts_stylus_data_v1>();

			const std::bitset<8> mode(data.mode);
			stylus.proximity = mode[IPTS_STYLUS_REPORT_MODE_BIT_PROXIMITY];
			stylus.contact = mode[IPTS_STYLUS_REPORT_MODE_BIT_CONTACT];
			stylus.button = mode[IPTS_STYLUS_REPORT_MODE_BIT_BUTTON];
			stylus.rubber = mode[IPTS_STYLUS_REPORT_MODE_BIT_RUBBER];

			stylus.x = data.x;
			stylus.y = data.y;
			stylus.pressure = data.pressure * 4;
			stylus.azimuth = 0;
			stylus.altitude = 0;
			stylus.timestamp = 0;
		}

		if (report.type == IPTS_REPORT_TYPE_STYLUS_V2) {
			const auto data = this->read<struct ipts_stylus_data_v2>();

			const std::bitset<16> mode(data.mode);
			stylus.proximity = mode[IPTS_STYLUS_REPORT_MODE_BIT_PROXIMITY];
			stylus.contact = mode[IPTS_STYLUS_REPORT_MODE_BIT_CONTACT];
			stylus.button = mode[IPTS_STYLUS_REPORT_MODE_BIT_BUTTON];
			stylus.rubber = mode[IPTS_STYLUS_REPORT_MODE_BIT_RUBBER];

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

void Parser::parse_heatmap(const struct ipts_payload_frame &frame)
{
	u32 size = 0;
	bool has_hm = false;
	bool has_dim = false;
	bool has_timestamp = false;

	struct ipts_heatmap_dim dim {};
	struct ipts_heatmap_timestamp time {};

	while (size < frame.size) {
		if (size + sizeof(struct ipts_report) > frame.size)
			break;

		const auto report = this->read<struct ipts_report>();
		size += sizeof(struct ipts_report);

		if (size + report.size > frame.size)
			break;

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

		size += report.size;
	}

	this->skip(frame.size - size);

	if (!has_hm)
		return;

	if (this->on_heatmap)
		this->on_heatmap(*this->heatmap);
}

void Parser::parse_heatmap_data(const struct ipts_heatmap_dim &dim,
				const struct ipts_heatmap_timestamp &time)
{
	if (this->heatmap) {
		if (this->heatmap->width != dim.width || this->heatmap->height != dim.height)
			this->heatmap.reset(nullptr);
	}

	if (!this->heatmap)
		this->heatmap = std::make_unique<Heatmap>(dim.width, dim.height);

	this->read(gsl::span(this->heatmap->data));

	this->heatmap->y_min = dim.y_min;
	this->heatmap->y_max = dim.y_max;
	this->heatmap->x_min = dim.x_min;
	this->heatmap->x_max = dim.x_max;
	this->heatmap->z_min = dim.z_min;
	this->heatmap->z_max = dim.z_max;

	this->heatmap->count = time.count;
	this->heatmap->timestamp = time.timestamp;
}

void Parser::parse_hid_heatmap(const struct ipts_data &header)
{
	const auto hid_header = this->read<struct ipts_hid_heatmap_header>();

	if (this->heatmap) {
		if (this->heatmap->size != hid_header.hm_size)
			this->heatmap.reset(nullptr);
	}

	if (!this->heatmap)
		this->heatmap = std::make_unique<Heatmap>(hid_header.hm_size);

	this->read(gsl::span(this->heatmap->data));

	this->parse_hid_heatmap_data();
	this->skip(header.size - hid_header.size - 7);
}

void Parser::parse_hid_heatmap_data()
{
	const auto frame_size = this->read<u32>();

	// The first three bytes contain bogus data, skip them.
	this->skip(3);
	u32 size = 3;

	bool has_dim = false;
	bool has_timestamp = false;

	while (size < frame_size) {
		if (size + sizeof(struct ipts_report) > frame_size)
			break;

		const auto report = this->read<struct ipts_report>();
		size += sizeof(struct ipts_report);

		if (size + report.size > frame_size)
			break;

		switch (report.type) {
		case IPTS_REPORT_TYPE_HEATMAP_TIMESTAMP: {
			const auto time = this->read<struct ipts_heatmap_timestamp>();

			this->heatmap->count = time.count;
			this->heatmap->timestamp = time.timestamp;

			has_timestamp = true;
			break;
		}
		case IPTS_REPORT_TYPE_HEATMAP_DIM: {
			const auto dim = this->read<struct ipts_heatmap_dim>();

			this->heatmap->height = dim.height;
			this->heatmap->width = dim.width;
			this->heatmap->y_min = dim.y_min;
			this->heatmap->y_max = dim.y_max;
			this->heatmap->x_min = dim.x_min;
			this->heatmap->x_max = dim.x_max;

			// These values are both 0 in the binary data, which
			// doesnt make sense. Lets use sane ones instead.
			this->heatmap->z_min = 0;
			this->heatmap->z_max = 255;

			has_dim = true;
			break;
		}
		default:
			this->skip(report.size);
			break;
		}

		size += report.size;
	}

	this->skip(frame_size - size);

	if (!has_dim || !has_timestamp)
		return;

	if (this->on_heatmap)
		this->on_heatmap(*this->heatmap);
}

} // namespace iptsd::ipts
