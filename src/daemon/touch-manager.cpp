// SPDX-License-Identifier: GPL-2.0-or-later

#include "touch-manager.hpp"

#include "config.hpp"

#include <common/types.hpp>
#include <contacts/processor.hpp>
#include <container/image.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/gsl>
#include <iterator>
#include <memory>
#include <spdlog/spdlog.h>
#include <utility>
#include <vector>

namespace iptsd::daemon {

TouchManager::TouchManager(Config conf)
	: size(), conf(conf), inputs(IPTS_MAX_CONTACTS), last(IPTS_MAX_CONTACTS),
	  distances(IPTS_MAX_CONTACTS * IPTS_MAX_CONTACTS)
{
	for (i32 i = 0; i < IPTS_MAX_CONTACTS; i++) {
		this->last[i].index = i;
		this->last[i].active = false;
	}

	this->processor.advanced = this->conf.touch_advanced;
	this->processor.conf.basic_pressure = this->conf.basic_pressure;
}

std::vector<TouchInput> &TouchManager::process(const ipts::Heatmap &data)
{
	this->processor.resize(index2_t {data.width, data.height});

	std::transform(data.data.begin(), data.data.end(), this->processor.hm().begin(),
		       [&](auto v) {
			       f32 val = static_cast<f32>(v - data.z_min) /
					 static_cast<f32>(data.z_max - data.z_min);

			       return 1.0f - val;
		       });

	const std::vector<contacts::TouchPoint> &contacts = this->processor.process();

	i32 count = std::min(gsl::narrow_cast<i32>(contacts.size()), static_cast<i32>(IPTS_MAX_CONTACTS));

	for (i32 i = 0; i < count; i++) {
		f64 x = contacts[i].mean.x;
		f64 y = contacts[i].mean.y;

		if (this->conf.invert_x)
			x = 1 - x;

		if (this->conf.invert_y)
			y = 1 - y;

		this->inputs[i].x = gsl::narrow_cast<i32>(x * IPTS_MAX_X);
		this->inputs[i].y = gsl::narrow_cast<i32>(y * IPTS_MAX_Y);

		math::Eigen2<f32> eigen = contacts[i].cov.eigen();
		f64 s1 = std::sqrt(eigen.w[0]);
		f64 s2 = std::sqrt(eigen.w[1]);

		f64 d1 = 4 * s1 / this->diagonal;
		f64 d2 = 4 * s2 / this->diagonal;

		f64 major = std::max(d1, d2);
		f64 minor = std::min(d1, d2);

		this->inputs[i].major = gsl::narrow_cast<i32>(major * IPTS_DIAGONAL);
		this->inputs[i].minor = gsl::narrow_cast<i32>(minor * IPTS_DIAGONAL);

		math::Vec2<f64> v1 = eigen.v[0].cast<f64>() * s1;
		f64 angle = M_PI_2 - std::atan2(v1.x, v1.y);

		// Make sure that the angle is always a positive number
		if (angle < 0)
			angle += M_PI;
		if (angle > M_PI)
			angle -= M_PI;

		this->inputs[i].orientation = gsl::narrow_cast<i32>(angle / M_PI * 180);
		this->inputs[i].palm = contacts[i].palm;

		this->inputs[i].ev1 = eigen.w[0];
		this->inputs[i].ev2 = eigen.w[1];

		this->inputs[i].index = i;
		this->inputs[i].active = true;
	}

	for (i32 i = count; i < IPTS_MAX_CONTACTS; i++) {
		this->inputs[i].index = i;
		this->inputs[i].active = false;
		this->inputs[i].palm = false;
		this->inputs[i].ev1 = 0;
		this->inputs[i].ev2 = 0;
	}

	this->track();

	if (this->conf.stylus_cone) {
		// Update touch rejection cones
		for (i32 i = 0; i < count; i++) {
			if (!this->inputs[i].palm)
				continue;

			this->update_cones(this->inputs[i]);
		}

		// Check if any contacts fall into the cones
		for (i32 i = 0; i < count; i++) {
			if (this->inputs[i].palm)
				continue;

			this->inputs[i].palm = this->check_cones(this->inputs[i]);
		}
	}

	std::swap(this->inputs, this->last);
	return this->last;
}

void TouchManager::track()
{
	// Calculate the distances between current and previous inputs
	for (u32 i = 0; i < IPTS_MAX_CONTACTS; i++) {
		for (u32 j = 0; j < IPTS_MAX_CONTACTS; j++) {
			const TouchInput &in = this->inputs[i];
			const TouchInput &last = this->last[j];

			u32 idx = i * IPTS_MAX_CONTACTS + j;

			// If one of the two inputs is / was not active, generate
			// a very high distance, so that the pair will only get chosen
			// if no "proper" pairs are left.
			if (!in.active || !last.active) {
				this->distances[idx] = (1 << 20) + idx;
				continue;
			}

			f64 dx = static_cast<f64>(in.x) - static_cast<f64>(last.x);
			f64 dy = static_cast<f64>(in.y) - static_cast<f64>(last.y);

			this->distances[idx] = std::sqrt(dx * dx + dy * dy);
		}
	}

	// Select the smallest calculated distance to find the closest two inputs.
	// Copy the index from the previous to the current input. Then invalidate
	// all distance entries that contain the two inputs, and repeat until we
	// found an index for all inputs.
	for (u32 k = 0; k < IPTS_MAX_CONTACTS; k++) {
		auto it = std::min_element(this->distances.begin(), this->distances.end());
		u32 idx = std::distance(this->distances.begin(), it);

		u32 i = idx / IPTS_MAX_CONTACTS;
		u32 j = idx % IPTS_MAX_CONTACTS;

		this->inputs[i].index = this->last[j].index;
		if (this->inputs[i].active)
			this->inputs[i].palm |= this->last[j].palm;

		f32 dev1 = this->inputs[i].ev1 - this->last[j].ev1;
		f32 dev2 = this->inputs[i].ev2 - this->last[j].ev2;

		this->inputs[i].stable = (dev1 < this->conf.stability_threshold &&
					  dev2 < this->conf.stability_threshold) ||
					 !this->conf.touch_stability;

		f64 dx = this->inputs[i].x - this->last[j].x;
		f64 dy = this->inputs[i].y - this->last[j].y;
		f64 sqdist = dx * dx + dy * dy;

		// Is the position stable?
		if (sqdist < this->conf.position_stability_threshold_square) {
			this->inputs[i].x = this->last[j].x;
			this->inputs[i].y = this->last[j].y;
		} else {
			f64 dist = std::sqrt(sqdist);
			f64 x = this->inputs[i].x -
				this->conf.position_stability_threshold * (dx / dist);
			f64 y = this->inputs[i].y -
				this->conf.position_stability_threshold * (dy / dist);

			this->inputs[i].x = gsl::narrow_cast<i32>(x);
			this->inputs[i].y = gsl::narrow_cast<i32>(y);
		}

		// Set the distance of all pairs that contain one of i and j
		// to something even higher than the distance chosen above.
		// This prevents i and j from getting selected again.
		for (u32 x = 0; x < IPTS_MAX_CONTACTS; x++) {
			u32 idx1 = i * IPTS_MAX_CONTACTS + x;
			u32 idx2 = x * IPTS_MAX_CONTACTS + j;

			this->distances[idx1] = (1 << 30) + idx1;
			this->distances[idx2] = (1 << 30) + idx2;
		}
	}
}

void TouchManager::update_cones(const TouchInput &palm)
{
	std::shared_ptr<Cone> cone {nullptr};
	f64 d = INFINITY;

	// find closest cone (by center)
	for (auto &current : this->cones) {
		// This cone has never seen a position update, so its inactive
		if (!current->alive())
			continue;

		if (!current->active())
			continue;

		f64 current_d = std::hypot(current->x - palm.x, current->y - palm.y);
		if (current_d < d) {
			d = current_d;
			cone = current;
		}
	}

	if (!cone)
		return;

	cone->update_direction(palm.x, palm.y);
}

bool TouchManager::check_cones(const TouchInput &input)
{
	for (const auto &cone : this->cones) {
		if (cone->check(input.x, input.y))
			return true;
	}

	return false;
}

} // namespace iptsd::daemon
