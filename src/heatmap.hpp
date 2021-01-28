/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_HEATMAP_HPP_
#define _IPTSD_HEATMAP_HPP_

#include <cstddef>
#include <cstdint>
#include <vector>

class Heatmap {
public:
	int32_t width;
	int32_t height;
	size_t size;
	int32_t touch_threshold;
	float diagonal;

	std::vector<uint8_t> data;
	std::vector<bool> visited;

	Heatmap(int32_t w, int32_t h, int32_t threshold);

	float average(void);
	uint8_t value(int32_t x, int32_t y);
	bool is_touch(int32_t x, int32_t y);
	bool compare(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
	bool get_visited(int32_t x, int32_t y);
	void set_visited(int32_t x, int32_t y, bool value);
};

#endif /* _IPTSD_HEATMAP_HPP_ */
