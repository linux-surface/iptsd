// SPDX-License-Identifier: GPL-2.0-or-later

#include "heatmap.hpp"

#include <common/types.hpp>

#include <cstddef>
#include <gsl/gsl>

namespace iptsd::contacts::basic {

f32 Heatmap::value(index2_t x)
{
	if (x < index2_t {0, 0} || x >= this->size)
		return 0;

	f32 val = this->data[x];
	if (val > this->average)
		return val - this->average;
	else
		return 0;
}

bool Heatmap::compare(index2_t x, index2_t y)
{
	f64 v1 = this->value(x);
	f64 v2 = this->value(y);

	if (v2 > v1)
		return false;

	if (v2 < v1)
		return true;

	if (y.x > x.x)
		return false;

	if (y.x < x.x)
		return true;

	if (y.y > x.y)
		return false;

	if (y.y < x.y)
		return true;

	return y.y == x.y;
}

bool Heatmap::get_visited(index2_t x)
{
	if (x < index2_t {0, 0} || x >= this->size)
		return false;

	return this->visited[x];
}

void Heatmap::set_visited(index2_t x, bool value)
{
	if (x < index2_t {0, 0} || x >= this->size)
		return;

	this->visited[x] = value;
}

void Heatmap::reset()
{
	for (index_t x = 0; x < this->size.x; x++) {
		for (index_t y = 0; y < this->size.y; y++)
			this->set_visited(index2_t {x, y}, false);
	}

	f32 value = 0;

	for (auto i : this->data)
		value += i;

	this->average = value / gsl::narrow_cast<f32>(this->size.span());
}

} // namespace iptsd::contacts::basic
