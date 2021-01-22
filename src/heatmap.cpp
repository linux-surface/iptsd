// SPDX-License-Identifier: GPL-2.0-or-later

#include "heatmap.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>

Heatmap::Heatmap(int32_t threshold)
{
	this->data = nullptr;
	this->visited = nullptr;
	this->width = 0;
	this->height = 0;
	this->size = 0;
	this->diagonal = 0;

	this->touch_threshold = threshold;
}

Heatmap::~Heatmap(void)
{
	if (this->data)
		delete[] this->data;

	if (this->visited)
		delete[] this->visited;
}

void Heatmap::resize(int32_t w, int32_t h)
{
	if (this->data)
		delete[] this->data;

	if (this->visited)
		delete[] this->visited;

	this->width = w;
	this->height = h;

	this->size = w * h;
	this->diagonal = std::sqrt(w * w + h * h);

	this->data = new uint8_t[this->size];
	this->visited = new bool[this->size];
}

float Heatmap::average(void)
{
	float value = 0;

	for (size_t i = 0; i < this->size; i++)
		value += this->data[i];

	return value / this->size;
}

uint8_t Heatmap::value(int32_t x, int32_t y)
{
	if (x < 0 || x >= this->width)
		return 0;

	if (y < 0 || y >= this->height)
		return 0;

	return this->data[y * this->width + x];
}

bool Heatmap::is_touch(int32_t x, int32_t y)
{
	return this->value(x, y) >= this->touch_threshold;
}

bool Heatmap::compare(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
	uint8_t v1 = this->value(x1, y1);
	uint8_t v2 = this->value(x2, y2);

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

bool Heatmap::get_visited(int32_t x, int32_t y)
{
	if (x < 0 || x >= this->width)
		return false;

	if (y < 0 || y >= this->height)
		return false;

	return this->visited[y * this->width + x];
}

void Heatmap::set_visited(int32_t x, int32_t y, bool value)
{
	if (x < 0 || x >= this->width)
		return;

	if (y < 0 || y >= this->height)
		return;

	this->visited[y * this->width + x] = value;
}
