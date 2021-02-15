/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONE_HPP_
#define _IPTSD_CONE_HPP_

#include <common/types.hpp>

#include <chrono>

using namespace std::chrono;

class Cone {
public:
	system_clock::time_point position_update;
	system_clock::time_point direction_update;
	f32 x;
	f32 y;
	f32 dx;
	f32 dy;

	Cone(void);

	bool was_active(void);
	void set_tip(f32 x, f32 y);
	bool is_removed(void);
	f32 hypot(f32 x, f32 y);
	void update_direction(f32 x, f32 y);
	bool is_inside(f32 x, f32 y);
};

#endif /* _IPTSD_CONE_HPP_ */
