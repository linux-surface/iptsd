// SPDX-License-Identifier: GPL-2.0-or-later

#include "heatmap.hpp"

#include <common/types.hpp>
#include <container/ops.hpp>

#include <algorithm>
#include <gsl/gsl>

namespace iptsd::contacts::basic {

f32 Heatmap::value(index2_t pos)
{
	index2_t size = this->data.size();

	if (pos.x < 0 || pos.x >= size.x)
		return 0;

	if (pos.y < 0 || pos.y >= size.y)
		return 0;

	return std::max(this->data[pos] - this->average, 0.0f);
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

	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++)
			this->set_visited(index2_t {x, y}, false);
	}

	this->average = container::ops::sum(this->data) / gsl::narrow<f32>(size.span());
}

} // namespace iptsd::contacts::basic
