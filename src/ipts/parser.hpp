/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_IPTS_PARSER_HPP
#define IPTSD_IPTS_PARSER_HPP

#include "protocol.hpp"
#include "reader.hpp"

#include <common/types.hpp>

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
};

class Heatmap {
public:
	u8 width = 0;
	u8 height = 0;
	u16 size = 0;

	u8 y_min = 0;
	u8 y_max = 0;
	u8 x_min = 0;
	u8 x_max = 0;
	u8 z_min = 0;
	u8 z_max = 0;
	u16 count = 0;
	u32 timestamp = 0;

	std::vector<u8> data;

	bool has_dim = false;
	bool has_time = false;
	bool has_data = false;
	bool has_size = false;

	void resize(u16 size);
	void resize(u8 w, u8 h);
};

class Parser {
private:
	std::unique_ptr<Heatmap> heatmap;

	void parse_raw(Reader &reader);
	void parse_hid(Reader &reader, u32 headersize);
	void parse_reports(Reader &reader, u32 framesize);

	void parse_stylus_v1(Reader &reader);
	void parse_stylus_v2(Reader &reader);

	void parse_heatmap_dim(Reader &reader);
	void parse_heatmap_timestamp(Reader &reader);
	void parse_heatmap_data(Reader &reader);
	void parse_heatmap_frame(Reader &reader);

	void try_submit_heatmap();

public:
	std::function<void(const StylusData &)> on_stylus;
	std::function<void(const Heatmap &)> on_heatmap;

	void parse(gsl::span<u8> data);
};

} /* namespace iptsd::ipts */

#endif /* IPTSD_IPTS_PARSER_HPP */
