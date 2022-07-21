/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_IPTS_PARSER_HPP
#define IPTSD_IPTS_PARSER_HPP

#include "protocol.hpp"
#include "reader.hpp"

#include <common/types.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <gsl/gsl>
#include <memory>
#include <optional>
#include <vector>

namespace iptsd::ipts {

class StylusData {
public:
	bool proximity = false;
	bool contact = false;
	bool button = false;
	bool rubber = false;

	u16 timestamp = 0;
	u16 x = 0;
	u16 y = 0;
	u16 pressure = 0;
	u16 altitude = 0;
	u16 azimuth = 0;
	u32 serial = 0;

	i32 real = 0;
	i32 imag = 0;
};

class Heatmap {
public:
	struct ipts_dimensions dim {};
	struct ipts_timestamp time {};

	std::vector<u8> data {};

	void resize(u16 size);
};

class DftWindow {
public:
	u8 rows = 0;
	u8 type = 0;

	struct ipts_dimensions dim {};
	struct ipts_timestamp time {};

	std::array<struct ipts_pen_dft_window_row, IPTS_DFT_MAX_ROWS> x {};
	std::array<struct ipts_pen_dft_window_row, IPTS_DFT_MAX_ROWS> y {};
};

class Parser {
private:
	std::unique_ptr<Heatmap> heatmap = nullptr;

	StylusData stylus {};
	struct ipts_dimensions dim {};
	struct ipts_timestamp time {};

	void parse_raw(Reader reader);
	void parse_hid(Reader reader);
	void parse_reports(Reader reader);

	void parse_stylus_v1(Reader reader);
	void parse_stylus_v2(Reader reader);

	void parse_dimensions(Reader reader);
	void parse_timestamp(Reader reader);
	void parse_heatmap_data(Reader reader);
	void parse_heatmap_frame(Reader reader);
	void parse_dft_window(Reader reader);

public:
	std::function<void(const StylusData &)> on_stylus;
	std::function<void(const Heatmap &)> on_heatmap;
	std::function<void(const DftWindow &, StylusData &)> on_dft;

	void parse(gsl::span<u8> data);
};

} /* namespace iptsd::ipts */

#endif /* IPTSD_IPTS_PARSER_HPP */
