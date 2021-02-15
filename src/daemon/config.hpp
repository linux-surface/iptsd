/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DAEMON_CONFIG_HPP_
#define _IPTSD_DAEMON_CONFIG_HPP_

#include <common/types.hpp>
#include <ipts/ipts.h>

#include <string>

class IptsdConfig {
public:
	bool invert_x;
	bool invert_y;

	i32 width;
	i32 height;

	bool block_on_palm;
	i32 touch_threshold;
	f32 stability_threshold;

	IptsdConfig(struct ipts_device_info info);

private:
	void load_dir(std::string name, struct ipts_device_info info);
};

#endif /* _IPTSD_DAEMON_CONFIG_HPP_ */
