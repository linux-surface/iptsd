/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_IPTS_PARSER_HPP_
#define _IPTSD_IPTS_PARSER_HPP_

#include "protocol.h"

#include <common/types.hpp>

#include <cstddef>
#include <functional>
#include <vector>

class IptsSingletouchData {
public:
	bool touch;
	u16 x;
	u16 y;
};

class IptsStylusData {
public:
	u16 timestamp;
	u16 mode;
	u16 x;
	u16 y;
	u16 pressure;
	u16 altitude;
	u16 azimuth;
	u32 serial;
};

class IptsHeatmap {
public:
	u8 width;
	u8 height;
	u8 y_min;
	u8 y_max;
	u8 x_min;
	u8 x_max;
	u8 z_min;
	u8 z_max;
	u16 size;

	std::vector<u8> data;

	IptsHeatmap(u8 width, u8 height);
};

class IptsParser {
private:
	u8 *data;
	size_t size;
	size_t current;

	IptsHeatmap *heatmap;
	IptsStylusData stylus;

	void read(void *dest, size_t size);
	void skip(size_t size);
	void reset(void);

	template <typename T> T read(void)
	{
		T value;
		this->read(&value, sizeof(value));
		return value;
	}

	void parse_payload(void);
	void parse_hid(void);

	void parse_stylus(struct ipts_payload_frame frame);
	void parse_stylus_report(struct ipts_report report);

	void parse_heatmap(struct ipts_payload_frame frame);
	void parse_heatmap_dim(void);

public:
	std::function<void(IptsSingletouchData)> on_singletouch;
	std::function<void(IptsStylusData)> on_stylus;
	std::function<void(IptsHeatmap)> on_heatmap;

	IptsParser(size_t size);
	~IptsParser(void);

	u8 *buffer();
	void parse();
};

#endif /* _IPTSD_IPTS_PARSER_HPP_ */
