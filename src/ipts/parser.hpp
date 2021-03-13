/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_IPTS_PARSER_HPP
#define IPTSD_IPTS_PARSER_HPP

#include "protocol.h"

#include <common/types.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <vector>

namespace iptsd::ipts {

class SingletouchData {
public:
	bool touch = false;
	u16 x = 0;
	u16 y = 0;
};

class StylusData {
public:
	u16 timestamp = 0;
	u16 mode = 0;
	u16 x = 0;
	u16 y = 0;
	u16 pressure = 0;
	u16 altitude = 0;
	u16 azimuth = 0;
	u32 serial = 0;
};

class Heatmap {
public:
	u8 width;
	u8 height;
	u16 size;

	u8 y_min = 0;
	u8 y_max = 0;
	u8 x_min = 0;
	u8 x_max = 0;
	u8 z_min = 0;
	u8 z_max = 0;
	u16 count = 0;
	u32 timestamp = 0;

	std::vector<u8> data;

	Heatmap(u8 w, u8 h) : width(w), height(h), size(w * h), data(size) {};
	Heatmap(u16 size) : width(0), height(0), size(size), data(size) {};
};

class Parser {
private:
	std::vector<u8> data;
	size_t index = 0;

	std::unique_ptr<Heatmap> heatmap;

	void read(const std::span<u8> dest);
	void skip(const size_t size);
	void reset();

	template <class T> T read();

	void parse_payload();
	void parse_hid(const struct ipts_data &header);

	void parse_singletouch();
	void parse_hid_heatmap(const struct ipts_data &header);
	void parse_hid_heatmap_data();

	void parse_stylus(const struct ipts_payload_frame &frame);
	void parse_stylus_report(const struct ipts_report &report);

	void parse_heatmap(const struct ipts_payload_frame &frame);
	void parse_heatmap_data(const struct ipts_heatmap_dim &dim,
				const struct ipts_heatmap_timestamp &time);

public:
	std::function<void(const SingletouchData &)> on_singletouch;
	std::function<void(const StylusData &)> on_stylus;
	std::function<void(const Heatmap &)> on_heatmap;

	Parser(size_t size) : data(size) {};

	const std::span<u8> buffer();
	void parse(bool reset = true);
	void parse_loop();
};

inline const std::span<u8> Parser::buffer()
{
	return std::span(this->data);
}

template <class T> inline T Parser::read()
{
	T value {};

	// We have to break type safety here, since all we have is a bytestream.
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	this->read(std::span(reinterpret_cast<u8 *>(&value), sizeof(value)));

	return value;
}

} /* namespace iptsd::ipts */

#endif /* IPTSD_IPTS_PARSER_HPP */
