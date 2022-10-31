// SPDX-License-Identifier: GPL-2.0-or-later

#include "heatmap.hpp"

#include <common/types.hpp>
#include <container/ops.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <gsl/gsl>
#include <iterator>
#include <limits>

namespace iptsd::contacts::basic {

f32 Heatmap::value(index2_t pos)
{
	index2_t size = this->data.size();

	if (pos.x < 0 || pos.x >= size.x)
		return 0;

	if (pos.y < 0 || pos.y >= size.y)
		return 0;

	f32 thresh = this->average + (8.0f / 255); // TODO: Magic number

	if (this->data[pos] < thresh)
		return 0.0f;

	return this->data[pos] - this->average;
}

bool Heatmap::compare(index2_t px, index2_t py)
{
	f64 x = this->value(px);
	f64 y = this->value(py);

	if (y > x)
		return false;

	if (y < x)
		return true;

	if (py.x > px.x)
		return false;

	if (py.x < px.x)
		return true;

	if (py.y > px.y)
		return false;

	if (py.y < px.y)
		return true;

	return py.y == px.y;
}

bool Heatmap::get_visited(index2_t pos)
{
	index2_t size = this->data.size();

	if (pos.x < 0 || pos.x >= size.x)
		return true;

	if (pos.y < 0 || pos.y >= size.y)
		return true;

	return this->visited[pos];
}

void Heatmap::set_visited(index2_t pos, bool value)
{
	index2_t size = this->data.size();

	if (pos.x < 0 || pos.x >= size.x)
		return;

	if (pos.y < 0 || pos.y >= size.y)
		return;

	this->visited[pos] = value;
}

void Heatmap::reset()
{
	index2_t size = this->data.size();
	std::array<u32, UINT8_MAX + 1> count {};

	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			this->set_visited(pos, false);

			u8 val = gsl::narrow_cast<u8>(this->data[pos] * UINT8_MAX);
			count.at(val)++;
		}
	}

	auto max = std::max_element(count.begin(), count.end());
	u32 idx = std::distance(count.begin(), max);

	this->average = gsl::narrow<f32>(idx + 1) / UINT8_MAX;
}

} // namespace iptsd::contacts::basic
