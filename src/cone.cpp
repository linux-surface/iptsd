// SPDX-License-Identifier: GPL-2.0-or-later

#include "cone.hpp"

#include "constants.hpp"

#include <chrono>
#include <cmath>

using namespace std::chrono;

Cone::Cone(void)
{
	this->direction_update = system_clock::from_time_t(0);
	this->position_update = system_clock::from_time_t(0);
}

bool Cone::was_active(void)
{
	return this->position_update > system_clock::from_time_t(0);
}

void Cone::set_tip(float x, float y)
{
	this->x = x;
	this->y = y;
	this->position_update = system_clock::now();
}

bool Cone::is_removed(void)
{
	return this->position_update + 300ms <= system_clock::now();
}

float Cone::hypot(float x, float y)
{
	return std::hypot(this->x - x, this->y - y);
}

void Cone::update_direction(float x, float y)
{
	system_clock::time_point timestamp = system_clock::now();

	auto time_diff = timestamp - this->direction_update;
	auto ms_diff = duration_cast<milliseconds>(time_diff);

	float weight = std::exp2f(-ms_diff.count() / 1000.0);
	float d = this->hypot(x, y);

	float dx = (x - this->x) / (d + 1E-6);
	float dy = (y - this->y) / (d + 1E-6);

	this->dx = weight * this->dx + dx;
	this->dy = weight * this->dy + dy;

	// Normalize cone direction vector
	d = std::hypotf(this->dx, this->dy) + 1E-6;
	this->dx /= d;
	this->dy /= d;

	this->direction_update = timestamp;
}

bool Cone::is_inside(float x, float y)
{
	if (this->is_removed())
		return false;

	float dx = x - this->x;
	float dy = y - this->y;
	float d = std::hypot(dx, dy);

	if (d > CONE_DISTANCE_THRESHOLD)
		return false;

	if (dx * this->dx + dy * this->dy > CONE_COS_THRESHOLD * d)
		return true;

	return false;
}
