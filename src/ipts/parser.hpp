// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PARSER_HPP
#define IPTSD_IPTS_PARSER_HPP

#include "data.hpp"
#include "protocol.hpp"

#include <common/casts.hpp>
#include <common/reader.hpp>
#include <common/types.hpp>

#include <gsl/gsl>

#include <array>
#include <bitset>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace iptsd::ipts {

class Parser {
public:
	// The callback that is invoked when stylus data was parsed.
	std::function<void(const StylusData &)> on_stylus;

	// The callback that is invoked when a capacitive heatmap was parsed.
	std::function<void(const Heatmap &)> on_heatmap;

	// The callback that is invoked when a DFT window was parsed.
	std::function<void(const DftWindow &)> on_dft;

	// The callback that is invoked when a metadata report was parsed.
	std::function<void(const Metadata &)> on_metadata;

private:
	struct ipts_dimensions m_dim {};
	struct ipts_timestamp m_time {};

public:
	/*!
	 * Parses IPTS touch data from a HID report buffer.
	 *
	 * The data must have a three byte header, consisting of the report ID and a timestamp.
	 *
	 * @param[in] data The data to parse.
	 */
	void parse(const gsl::span<u8> data)
	{
		this->parse<struct ipts_header>(data);
	}

	/*!
	 * Parses IPTS touch data with an arbitrary header.
	 *
	 * @tparam T The type (and size) of the header.
	 * @param[in] data The data to parse.
	 */
	template <class T>
	void parse(const gsl::span<u8> data)
	{
		this->parse_with_header(data, sizeof(T));
	}

private:
	void parse_with_header(const gsl::span<u8> data, const usize header)
	{
		Reader reader(data);
		reader.skip(header);

		this->parse_frame(reader);
	}

	/*!
	 * Parses the root HID frame.
	 *
	 * On newer devices, a touch data report will contain a single HID frame containing other
	 * HID frames. On older devices this is emulated, by sending a single HID frame with a
	 * custom data type. Instead of more HID frames, this frame will contain the raw data
	 * from the IPTS device.
	 *
	 * @param[in] reader The chunk of data allocated to the root frame.
	 */
	void parse_frame(Reader &reader)
	{
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
		default:
			// TODO: Add handler for unknow data and wire up debug tools
			break;
		}
	}

	/*!
	 * Parses IPTS raw data.
	 *
	 * This data is found on older IPTS devices, who don't natively support HID.
	 * The data will contain a header, followed by multiple frames. Each frame identifies
	 * a different family of data (e.g. heatmap or stylus data) and consists of multiple
	 * reports.
	 *
	 * @param[in] reader The chunk of data allocated to the raw data.
	 */
	void parse_raw(Reader &reader)
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
			default:
				// TODO: Add handler for unknow data and wire up debug tools
				break;
			}
		}
	}

	/*!
	 * Parses IPTS HID frames.
	 *
	 * This data is found on newer IPTS devices that natively support HID.
	 * There can be multiple HID frames chained together. Each frame contains
	 * a certain type of content, how it is structured depends on the frame type.
	 *
	 * @param[in] reader The chunk of data allocated to the HID frame.
	 */
	void parse_hid(Reader &reader)
	{
		while (reader.size() > 0) {
			const auto frame = reader.read<struct ipts_hid_frame>();
			Reader sub = reader.sub(frame.size - sizeof(frame));

			switch (frame.type) {
			case IPTS_HID_FRAME_TYPE_HEATMAP:
				this->parse_heatmap_frame(sub);
				break;
			case IPTS_HID_FRAME_TYPE_REPORTS:
				/*
				 * On SP7 we receive the following data about once per second:
				 * 16 00 00 00 00 00 00
				 *   0b 00 00 00 00 ff 00
				 *     74 00 04 00 00 00 00 00
				 * This causes a parse error, because the "0b" should be "0f".
				 * So let's just ignore these packets.
				 */
				if (reader.size() == 4)
					return;

				this->parse_reports(sub);
				break;
			case IPTS_HID_FRAME_TYPE_METADATA:
				this->parse_metadata(sub);
				break;
			default:
				// TODO: Add handler for unknow data and wire up debug tools
				break;
			}
		}
	}

	/*!
	 * Parses an IPTS metadata frame.
	 *
	 * This data is only found on devices that natively support HID and has to be retrieved
	 * through a HID feature report.
	 *
	 * Once the data is parsed, the @ref on_metadata calback is invoked.
	 *
	 * @param[in] reader The chunk of data allocated to the metadata frame.
	 */
	void parse_metadata(Reader &reader) const
	{
		Metadata m {};

		m.size = reader.read<struct ipts_touch_metadata_size>();
		m.unknown_byte = reader.read<u8>();
		m.transform = reader.read<struct ipts_touch_metadata_transform>();
		m.unknown = reader.read<struct ipts_touch_metadata_unknown>();

		if (this->on_metadata)
			this->on_metadata(m);
	}

	/*!
	 * Parses IPTS reports.
	 *
	 * Reports can be found on both types of devices (both HID-native and not).
	 * They are found inside of a frame structure and describe different aspects
	 * of the data family described by the frame. The frame contains no indication
	 * about the amount of reports, only their combined size.
	 *
	 * @param[in] reader The chunk of data allocated to the list of reports.
	 */
	void parse_reports(Reader &reader)
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
			default:
				// TODO: Add handler for unknow data and wire up debug tools
				break;
			}
		}
	}

	/*!
	 * Parses a first generation stylus report.
	 *
	 * These reports are found on devices with a stylus that does not
	 * support tilt information, and that only supports 1024 levels of pressure.
	 *
	 * The stylus report can contain multiple elements, each describing a different
	 * sample of the stylus state and position from a 5 millisecond window.
	 * For the last element, the @ref on_stylus callback will be invoked. The other
	 * elements are dropped, to prevent jitter in the output. The 1024 pressure levels
	 *  will be scaled to the same 4096 levels that newer devices support.
	 *
	 * @param[in] reader The chunk of data allocated to the report.
	 */
	void parse_stylus_v1(Reader &reader) const
	{
		StylusData stylus;

		const auto stylus_report = reader.read<struct ipts_stylus_report>();
		stylus.serial = stylus_report.serial;

		for (u8 i = 0; i < stylus_report.elements - 1; i++)
			reader.skip(sizeof(struct ipts_stylus_data_v1));

		const auto data = reader.read<struct ipts_stylus_data_v1>();

		const std::bitset<8> mode {data.mode};
		stylus.proximity = mode[IPTS_STYLUS_REPORT_MODE_BIT_PROXIMITY];
		stylus.button = mode[IPTS_STYLUS_REPORT_MODE_BIT_BUTTON];
		stylus.rubber = mode[IPTS_STYLUS_REPORT_MODE_BIT_RUBBER];

		stylus.x = casts::to<f64>(data.x) / IPTS_MAX_X;
		stylus.y = casts::to<f64>(data.y) / IPTS_MAX_Y;
		stylus.pressure = casts::to<f64>(data.pressure) / IPTS_MAX_PRESSURE_V1;
		stylus.azimuth = 0;
		stylus.altitude = 0;
		stylus.timestamp = 0;

		stylus.contact = stylus.pressure > 0;

		if (this->on_stylus)
			this->on_stylus(stylus);
	}

	/*!
	 * Parses a second generation stylus report.
	 *
	 * These reports are found on devices with a stylus with support for tilt
	 * and 4096 levels of pressure.
	 *
	 * The stylus report can contain multiple elements, each describing a different
	 * sample of the stylus state and position from a 5 millisecond window.
	 * For the last element, the @ref on_stylus callback will be invoked. The other
	 * elements are dropped, to prevent jitter in the output.
	 *
	 * @param[in] reader The chunk of data allocated to the report.
	 */
	void parse_stylus_v2(Reader &reader) const
	{
		StylusData stylus;

		const auto stylus_report = reader.read<struct ipts_stylus_report>();
		stylus.serial = stylus_report.serial;

		for (u8 i = 0; i < stylus_report.elements - 1; i++)
			reader.skip(sizeof(struct ipts_stylus_data_v2));

		const auto data = reader.read<struct ipts_stylus_data_v2>();

		const std::bitset<16> mode(data.mode);
		stylus.proximity = mode[IPTS_STYLUS_REPORT_MODE_BIT_PROXIMITY];
		stylus.button = mode[IPTS_STYLUS_REPORT_MODE_BIT_BUTTON];
		stylus.rubber = mode[IPTS_STYLUS_REPORT_MODE_BIT_RUBBER];

		stylus.x = casts::to<f64>(data.x) / IPTS_MAX_X;
		stylus.y = casts::to<f64>(data.y) / IPTS_MAX_Y;
		stylus.pressure = casts::to<f64>(data.pressure) / IPTS_MAX_PRESSURE_V2;
		stylus.timestamp = data.timestamp;

		stylus.azimuth = casts::to<f64>(data.azimuth) / 18000.0 * M_PI;
		stylus.altitude = casts::to<f64>(data.altitude) / 18000.0 * M_PI;

		stylus.contact = stylus.pressure > 0;

		if (this->on_stylus)
			this->on_stylus(stylus);
	}

	/*!
	 * Parses a heatmap dimensions report.
	 *
	 * This report describes the scale and size of a heatmap sent by IPTS.
	 * While IPTS will send a heatmap report and a dimensions report in the same
	 * frame, the size of the heatmap is defined by the *previous* dimensions report.
	 *
	 * This means that the dimensions have to be cached until the full heatmap has been parsed.
	 *
	 * @param[in] reader The chunk of data allocated to the report.
	 */
	void parse_dimensions(Reader &reader)
	{
		m_dim = reader.read<struct ipts_dimensions>();

		// On newer devices, z_max may be 0, lets use a sane value instead.
		if (m_dim.z_max == 0)
			m_dim.z_max = 255;
	}

	/*!
	 * Parses a heatmap timestamp report.
	 *
	 * @param[in] reader The chunk of data allocated to the report.
	 */
	void parse_timestamp(Reader &reader)
	{
		m_time = reader.read<struct ipts_timestamp>();
	}

	/*!
	 * Parses a heatmap report.
	 *
	 * This report contains the actual heatmap data. IPTS sends
	 * heatmaps "inverted", e.g. a contact is represented by a low
	 * value, and no contact is represented by a high value.
	 *
	 * After the data was parsed, the @ref on_heatmap callback will be invoked.
	 *
	 * @param[in] reader The chunk of data allocated to the report.
	 */
	void parse_heatmap_data(Reader &reader)
	{
		Heatmap heatmap {};

		const usize size = casts::to<usize>(m_dim.width) * m_dim.height;

		heatmap.data = reader.subspan(size);
		heatmap.dim = m_dim;
		heatmap.time = m_time;

		if (this->on_heatmap)
			this->on_heatmap(heatmap);
	}

	/*!
	 * Parses a heatmap frame.
	 *
	 * On HID-native devices, the heatmap is not passed as a report. Instead it is passed
	 * inside of a HID frame with a custom frame and header.
	 *
	 * @param[in] reader The chunk of data allocated to the frame.
	 */
	void parse_heatmap_frame(Reader &reader)
	{
		const auto header = reader.read<struct ipts_heatmap_header>();
		Reader sub = reader.sub(header.size);

		this->parse_heatmap_data(sub);
	}

	/*!
	 * Parses a DFT window report.
	 *
	 * HID-native devices use DFT based pens. Instead of doing processing in firmware,
	 * and returning readily usable coordinates, these devices will return a set of
	 * antenna measurements and leave it to the client to determine the exact position
	 * of the stylus.
	 *
	 * After the data was parsed, the @ref on_dft callback will be invoked.

	 * @param[in] reader The chunk of data allocated to the report.
	 */
	void parse_dft_window(Reader &reader)
	{
		DftWindow dft {};
		const auto window = reader.read<struct ipts_pen_dft_window>();

		for (usize i = 0; i < window.num_rows; i++)
			dft.x.at(i) = reader.read<struct ipts_pen_dft_window_row>();

		for (usize i = 0; i < window.num_rows; i++)
			dft.y.at(i) = reader.read<struct ipts_pen_dft_window_row>();

		dft.rows = window.num_rows;
		dft.type = window.data_type;

		dft.dim = m_dim;
		dft.time = m_time;

		if (!this->on_dft)
			return;

		this->on_dft(dft);
	}
};

} // namespace iptsd::ipts

#endif // IPTSD_IPTS_PARSER_HPP
