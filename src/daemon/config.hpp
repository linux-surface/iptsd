/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DAEMON_CONFIG_HPP_
#define _IPTSD_DAEMON_CONFIG_HPP_

#include <common/types.hpp>
#include <ipts/ipts.h>

#include <string>

class IptsdConfig {
public:
	bool invert_x = false;
	bool invert_y = false;

	i32 width = 0;
	i32 height = 0;

	struct ipts_device_info info;

	IptsdConfig(struct ipts_device_info info);

private:
	void load_dir(const std::string &name);
};

#endif /* _IPTSD_DAEMON_CONFIG_HPP_ */
