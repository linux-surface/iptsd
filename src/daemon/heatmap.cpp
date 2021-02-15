// SPDX-License-Identifier: GPL-2.0-or-later

#include "heatmap.hpp"

#include "types.hpp"

#include <cstddef>

Heatmap::Heatmap(i32 w, i32 h, i32 threshold) : data(w * h), visited(w * h)
{
	this->width = w;
	this->height = h;

	this->size = w * h;
	this->diagonal = std::sqrt(w * w + h * h);

	this->touch_threshold = threshold;
}

f32 Heatmap::average(void)
{
	f32 value = 0;

	for (size_t i = 0; i < this->size; i++)
		value += this->data[i];

	return value / this->size;
}

u8 Heatmap::value(i32 x, i32 y)
{
	if (x < 0 || x >= this->width)
		return 0;

	if (y < 0 || y >= this->height)
		return 0;

	return this->data[y * this->width + x];
}

bool Heatmap::is_touch(i32 x, i32 y)
{
	return this->value(x, y) >= this->touch_threshold;
}

bool Heatmap::compare(i32 x1, i32 y1, i32 x2, i32 y2)
{
	u8 v1 = this->value(x1, y1);
	u8 v2 = this->value(x2, y2);

	if (v2 > v1)
		return false;

	if (v2 < v1)
		return true;

	if (x2 > x1)
		return false;

	if (x2 < x1)
		return true;

	if (y2 > y1)
		return false;

	if (y2 < y1)
		return true;

	return y2 == y1;
}

bool Heatmap::get_visited(i32 x, i32 y)
{
	if (x < 0 || x >= this->width)
		return false;

	if (y < 0 || y >= this->height)
		return false;

	return this->visited[y * this->width + x];
}

void Heatmap::set_visited(i32 x, i32 y, bool value)
{
	if (x < 0 || x >= this->width)
		return;

	if (y < 0 || y >= this->height)
		return;

	this->visited[y * this->width + x] = value;
}
