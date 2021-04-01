/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_CONFIG_HPP
#define IPTSD_DAEMON_CONFIG_HPP

#include <common/types.hpp>
#include <ipts/ipts.h>

#include <string>

namespace iptsd::daemon {

class Config {
public:
	bool invert_x = false;
	bool invert_y = false;

	i32 width = 0;
	i32 height = 0;

	bool disable_touch_on_stylus = false;

	struct ipts_device_info info;

	Config(struct ipts_device_info info);

private:
	void load_dir(const std::string &name);
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_CONFIG_HPP */
