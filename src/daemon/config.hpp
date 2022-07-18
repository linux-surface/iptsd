/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_CONFIG_HPP
#define IPTSD_DAEMON_CONFIG_HPP

#include <common/types.hpp>

#include <optional>
#include <string>

namespace iptsd::daemon {

class Config {
public:
	bool invert_x = false;
	bool invert_y = false;

	i32 width = 0;
	i32 height = 0;

	bool stylus_cone = true;
	bool stylus_disable_touch = false;

	bool touch_stability = true;
	bool touch_advanced = false;
	bool touch_disable_on_palm = false;

	f32 basic_pressure = 0.04;

	f32 cone_angle = 30;
	f32 cone_distance = 1600;

	f32 stability_threshold = 0.1;
	f32 position_stability_threshold = 8;
	f64 position_stability_threshold_square = 8 * 8;

	i16 vendor;
	i16 product;

	Config(i16 vendor, i16 product);

private:
	void load_dir(const std::string &name);
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_CONFIG_HPP */
