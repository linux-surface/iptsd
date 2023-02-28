// SPDX-License-Identifier: GPL-2.0-or-later

#include "cone.hpp"

#include <common/types.hpp>

#include <chrono>
#include <cmath>
#include <gsl/gsl>

namespace iptsd::daemon {

bool Cone::alive()
{
	return this->position_update > clock::from_time_t(0);
}

void Cone::update_position(f64 x, f64 y)
{
	this->x = x;
	this->y = y;
	this->position_update = clock::now();
}

bool Cone::active()
{
	return this->position_update + std::chrono::milliseconds(300) > clock::now();
}

void Cone::update_direction(f64 x, f64 y)
{
	const clock::time_point timestamp = clock::now();

	const auto time_diff = timestamp - this->direction_update;
	const auto diff = std::chrono::duration_cast<std::chrono::seconds>(time_diff);

	const f64 weight = std::exp2(-diff.count());
	f64 dist = std::hypot(this->x - x, this->y - y);

	const f64 dx = (x - this->x) / (dist + 1E-6);
	const f64 dy = (y - this->y) / (dist + 1E-6);

	this->dx = weight * this->dx + dx;
	this->dy = weight * this->dy + dy;

	// Normalize cone direction vector
	dist = std::hypot(this->dx, this->dy) + 1E-6;
	this->dx /= dist;
	this->dy /= dist;

	this->direction_update = timestamp;
}

bool Cone::check(f64 x, f64 y)
{
	if (!this->active())
		return false;

	const f64 dx = x - this->x;
	const f64 dy = y - this->y;
	const f64 dist = std::hypot(dx, dy);

	if (dist > this->distance)
		return false;

	if (dx * this->dx + dy * this->dy > this->angle * dist)
		return true;

	return false;
}

} // namespace iptsd::daemon
