/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_HEATMAP_HPP
#define IPTSD_CONTACTS_BASIC_HEATMAP_HPP

#include <common/types.hpp>
#include <container/image.hpp>

#include <cstddef>
#include <vector>

namespace iptsd::contacts::basic {

class Heatmap {
public:
	index2_t size;
	f64 diagonal;
	f32 average = 0;

	container::Image<f32> data;
	container::Image<bool> visited;

	Heatmap(index2_t size)
		: size(size), diagonal(std::sqrt(size.x * size.x + size.y * size.y)), data(size),
		  visited(size) {};

	f32 value(index2_t x);
	bool compare(index2_t x, index2_t y);
	bool get_visited(index2_t x);
	void set_visited(index2_t x, bool value);
	void reset();
};

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_HEATMAP_HPP */
