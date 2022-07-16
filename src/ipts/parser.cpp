// SPDX-License-Identifier: GPL-2.0-or-later

#include "parser.hpp"

#include "protocol.h"
#include "reader.hpp"

#include <common/access.hpp>
#include <common/types.hpp>

#include <bitset>
#include <cstring>
#include <gsl/gsl>
#include <gsl/span>
#include <memory>
#include <stdexcept>
#include <utility>

namespace iptsd::ipts {

void Heatmap::resize(u16 size)
{
	this->width = 0;
	this->height = 0;
	this->size = size;

	if (this->data.size() != this->size)
		this->data.resize(this->size);
}

void Heatmap::resize(u8 w, u8 h)
{
	this->resize(w * h);
	this->width = w;
	this->height = h;
}

void Parser::parse(const gsl::span<u8> data)
{
	Reader reader(data);

	// Read the report header
	const auto header = reader.read<struct ipts_header>();

	// Check if we are dealing with GuC based or HID based IPTS
	if (header.timestamp == 0xFFFF)
		this->parse_raw(reader);
	else
		this->parse_hid(reader);
}

void Parser::parse_raw(Reader &reader)
{
	const auto header = reader.read<struct ipts_raw_header>();

	for (u32 i = 0; i < header.frames; i++) {
		const auto frame = reader.read<struct ipts_raw_frame>();

		switch (frame.type) {
		case IPTS_RAW_FRAME_TYPE_STYLUS:
		case IPTS_RAW_FRAME_TYPE_HEATMAP:
			this->parse_reports(reader, frame.size);
			break;
		default:
			reader.skip(frame.size);
			break;
		}
	}
}

void Parser::parse_hid(Reader &reader)
{
	u32 size = 0;
	const auto header = reader.read<struct ipts_hid_frame>();

	while (size < header.size) {
		const auto frame = reader.read<struct ipts_hid_frame>();
		size += frame.size;

		switch (frame.type) {
		case IPTS_HID_FRAME_TYPE_HEATMAP:
			this->parse_heatmap_frame(reader);
			break;
		case IPTS_HID_FRAME_TYPE_REPORTS:
			this->parse_reports(reader, frame.size - sizeof(frame));
			break;
		default:
			reader.skip(frame.size - sizeof(frame));
			break;
		}
	}
}

void Parser::parse_reports(Reader &reader, u32 framesize)
{
	u32 size = 0;

	while (size < framesize) {
		if (size + sizeof(struct ipts_report) > framesize)
			break;

		const auto report = reader.read<struct ipts_report>();
		size += sizeof(struct ipts_report);

		if (size + report.size > framesize)
			break;

		switch (report.type) {
		case IPTS_REPORT_TYPE_STYLUS_V1:
			this->parse_stylus_v1(reader);
			break;
		case IPTS_REPORT_TYPE_STYLUS_V2:
			this->parse_stylus_v2(reader);
			break;
		case IPTS_REPORT_TYPE_HEATMAP_DIM:
			this->parse_heatmap_dim(reader);
			break;
		case IPTS_REPORT_TYPE_HEATMAP_TIMESTAMP:
			this->parse_heatmap_timestamp(reader);
			break;
		case IPTS_REPORT_TYPE_HEATMAP:
			this->parse_heatmap_data(reader);
			break;
		default:
			reader.skip(report.size);
			break;
		}

		size += report.size;
	}

	reader.skip(framesize - size);
}

void Parser::parse_stylus_v1(Reader &reader)
{
	StylusData stylus;

	const auto stylus_report = reader.read<struct ipts_stylus_report>();
	stylus.serial = stylus_report.serial;

	for (u8 i = 0; i < stylus_report.elements; i++) {
		const auto data = reader.read<struct ipts_stylus_data_v1>();

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

		if (this->on_stylus)
			this->on_stylus(stylus);
	}
}

void Parser::parse_stylus_v2(Reader &reader)
{
	StylusData stylus;

	const auto stylus_report = reader.read<struct ipts_stylus_report>();
	stylus.serial = stylus_report.serial;

	for (u8 i = 0; i < stylus_report.elements; i++) {
		const auto data = reader.read<struct ipts_stylus_data_v2>();

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

		if (this->on_stylus)
			this->on_stylus(stylus);
	}
}

void Parser::parse_heatmap_dim(Reader &reader)
{
	auto const dim = reader.read<struct ipts_heatmap_dim>();

	if (!this->heatmap)
		this->heatmap = std::make_unique<Heatmap>();

	this->heatmap->resize(dim.width, dim.height);

	this->heatmap->y_min = dim.y_min;
	this->heatmap->y_max = dim.y_max;
	this->heatmap->x_min = dim.x_min;
	this->heatmap->x_max = dim.x_max;
	this->heatmap->z_min = dim.z_min;
	this->heatmap->z_max = dim.z_max;

	// On newer devices, z_max may be 0, lets use a sane value instead.
	if (this->heatmap->z_max == 0)
		this->heatmap->z_max = 255;

	this->heatmap->has_dim = true;
	this->heatmap->has_size = true;
	this->try_submit_heatmap();
}

void Parser::parse_heatmap_timestamp(Reader &reader)
{
	auto const timestamp = reader.read<struct ipts_heatmap_timestamp>();

	if (!this->heatmap)
		this->heatmap = std::make_unique<Heatmap>();

	this->heatmap->count = timestamp.count;
	this->heatmap->timestamp = timestamp.timestamp;

	this->heatmap->has_time = true;
	this->try_submit_heatmap();
}

void Parser::parse_heatmap_data(Reader &reader)
{
	if (!this->heatmap)
		return;

	if (!this->heatmap->has_size)
		return;

	reader.read(gsl::span(this->heatmap->data));

	this->heatmap->has_data = true;
	this->try_submit_heatmap();
}

void Parser::parse_heatmap_frame(Reader &reader)
{
	const auto header = reader.read<struct ipts_heatmap_header>();

	if (!this->heatmap)
		this->heatmap = std::make_unique<Heatmap>();

	this->heatmap->resize(header.size);
	this->parse_heatmap_data(reader);
}

void Parser::try_submit_heatmap()
{
	if (!this->heatmap->has_dim)
		return;

	if (!this->heatmap->has_time)
		return;

	if (!this->heatmap->has_size)
		return;

	if (!this->heatmap->has_data)
		return;

	if (this->on_heatmap)
		this->on_heatmap(*this->heatmap);

	this->heatmap->has_dim = false;
	this->heatmap->has_time = false;
	this->heatmap->has_data = false;
	this->heatmap->has_size = false;
}

} // namespace iptsd::ipts
