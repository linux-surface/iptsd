/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_HEATMAP_HPP_
#define _IPTSD_HEATMAP_HPP_

#include <common/types.hpp>

#include <cstddef>
#include <vector>

class Heatmap {
public:
	i32 width;
	i32 height;
	size_t size;
	i32 touch_threshold;
	f32 diagonal;

	std::vector<u8> data;
	std::vector<bool> visited;

	Heatmap(i32 w, i32 h, i32 threshold);

	f32 average(void);
	u8 value(i32 x, i32 y);
	bool is_touch(i32 x, i32 y);
	bool compare(i32 x1, i32 y1, i32 x2, i32 y2);
	bool get_visited(i32 x, i32 y);
	void set_visited(i32 x, i32 y, bool value);
};

#endif /* _IPTSD_HEATMAP_HPP_ */
