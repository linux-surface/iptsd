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
	container::Image<f32> data;

private:
	f32 average = 0;
	container::Image<bool> visited;

public:
	Heatmap(index2_t size) : data {size}, visited {size} {};

	f32 value(index2_t pos);
	bool compare(index2_t px, index2_t py);
	bool get_visited(index2_t pos);
	void set_visited(index2_t pos, bool value);
	void reset();
};

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_HEATMAP_HPP */
