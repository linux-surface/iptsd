/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_CONE_HPP
#define IPTSD_DAEMON_CONE_HPP

#include <common/types.hpp>
#include <math/num.hpp>

#include <chrono>

namespace iptsd::daemon {

class Cone {
public:
	using clock = std::chrono::system_clock;

	clock::time_point position_update {};
	clock::time_point direction_update {};

	f64 x = 0;
	f64 y = 0;
	f64 dx = 0;
	f64 dy = 0;

	f32 angle;
	f32 distance;

public:
	Cone(f32 angle, f32 distance)
		: angle {std::cos(angle / 180 * math::num<f32>::pi)}, distance {distance} {};

	bool alive();
	bool active();

	void update_position(f64 x, f64 y);
	void update_direction(f64 x, f64 y);

	bool check(f64 x, f64 y);
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_CONE_HPP */
