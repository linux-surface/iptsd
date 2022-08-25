// SPDX-License-Identifier: GPL-2.0-or-later

#include "parser.hpp"

#include "protocol.hpp"
#include "reader.hpp"

#include <common/types.hpp>

#include <bitset>
#include <cstring>
#include <gsl/gsl>
#include <memory>
#include <stdexcept>
#include <utility>

namespace iptsd::ipts {

void Heatmap::resize(u16 size)
{
	if (this->data.size() != size)
		this->data.resize(size);
}

void Parser::parse(const gsl::span<u8> data)
{
	Reader reader(data);
	reader.skip(sizeof(struct ipts_header));

	const auto header = reader.read<struct ipts_hid_frame>();
	Reader sub = reader.sub(header.size - sizeof(header));

	// Check if we are dealing with GuC based or HID based IPTS
	switch (header.type) {
	case IPTS_HID_FRAME_TYPE_RAW:
		this->parse_raw(sub);
		break;
	case IPTS_HID_FRAME_TYPE_HID:
		this->parse_hid(sub);
		break;
	}
}

void Parser::parse_raw(Reader reader)
{
	const auto header = reader.read<struct ipts_raw_header>();

	for (u32 i = 0; i < header.frames; i++) {
		const auto frame = reader.read<struct ipts_raw_frame>();
		Reader sub = reader.sub(frame.size);

		switch (frame.type) {
		case IPTS_RAW_FRAME_TYPE_STYLUS:
		case IPTS_RAW_FRAME_TYPE_HEATMAP:
			this->parse_reports(sub);
			break;
		}
	}
}

void Parser::parse_hid(Reader reader)
{
	while (reader.size() > 0) {
		const auto frame = reader.read<struct ipts_hid_frame>();
		Reader sub = reader.sub(frame.size - sizeof(frame));

		switch (frame.type) {
		case IPTS_HID_FRAME_TYPE_HEATMAP:
			this->parse_heatmap_frame(sub);
			break;
		case IPTS_HID_FRAME_TYPE_REPORTS:
			this->parse_reports(sub);
			break;
		}
	}
}

void Parser::parse_reports(Reader reader)
{
	while (reader.size() > 0) {
		const auto report = reader.read<struct ipts_report>();
		Reader sub = reader.sub(report.size);

		switch (report.type) {
		case IPTS_REPORT_TYPE_STYLUS_V1:
			this->parse_stylus_v1(sub);
			break;
		case IPTS_REPORT_TYPE_STYLUS_V2:
			this->parse_stylus_v2(sub);
			break;
		case IPTS_REPORT_TYPE_DIMENSIONS:
			this->parse_dimensions(sub);
			break;
		case IPTS_REPORT_TYPE_TIMESTAMP:
			this->parse_timestamp(sub);
			break;
		case IPTS_REPORT_TYPE_HEATMAP:
			this->parse_heatmap_data(sub);
			break;
		case IPTS_REPORT_TYPE_PEN_DFT_WINDOW:
			this->parse_dft_window(sub);
			break;
		}
	}
}

void Parser::parse_stylus_v1(Reader reader)
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

void Parser::parse_stylus_v2(Reader reader)
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

void Parser::parse_dimensions(Reader reader)
{
	this->dim = reader.read<struct ipts_dimensions>();

	// On newer devices, z_max may be 0, lets use a sane value instead.
	if (this->dim.z_max == 0)
		this->dim.z_max = 255;
}

void Parser::parse_timestamp(Reader reader)
{
	this->time = reader.read<struct ipts_timestamp>();
}

void Parser::parse_heatmap_data(Reader reader)
{
	if (!this->heatmap)
		this->heatmap = std::make_unique<Heatmap>();

	this->heatmap->resize(this->dim.width * this->dim.height);

	reader.read(gsl::span(this->heatmap->data));

	this->heatmap->dim = this->dim;
	this->heatmap->time = this->time;

	if (this->on_heatmap)
		this->on_heatmap(*this->heatmap);
}

void Parser::parse_heatmap_frame(Reader reader)
{
	const auto header = reader.read<struct ipts_heatmap_header>();
	Reader sub = reader.sub(header.size);

	this->parse_heatmap_data(sub);
}

void Parser::parse_dft_window(Reader reader)
{
	DftWindow dft {};
	const auto window = reader.read<struct ipts_pen_dft_window>();

	for (int i = 0; i < window.num_rows; i++)
		dft.x.at(i) = reader.read<struct ipts_pen_dft_window_row>();

	for (int i = 0; i < window.num_rows; i++)
		dft.y.at(i) = reader.read<struct ipts_pen_dft_window_row>();

	dft.rows = window.num_rows;
	dft.type = window.data_type;

	dft.dim = this->dim;
	dft.time = this->time;

	if (!this->on_dft)
		return;

	this->on_dft(dft, this->stylus);

	if (!this->on_stylus)
		return;

	this->on_stylus(this->stylus);
}

} // namespace iptsd::ipts
