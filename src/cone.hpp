/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONE_HPP_
#define _IPTSD_CONE_HPP_

#include <chrono>

using namespace std::chrono;

class Cone {
public:
	system_clock::time_point position_update;
	system_clock::time_point direction_update;
	float x;
	float y;
	float dx;
	float dy;

	Cone(void);

	bool was_active(void);
	void set_tip(float x, float y);
	bool is_removed(void);
	float hypot(float x, float y);
	void update_direction(float x, float y);
	bool is_inside(float x, float y);
};

#endif /* _IPTSD_CONE_HPP_ */
