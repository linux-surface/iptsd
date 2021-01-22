/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONFIG_HPP_
#define _IPTSD_CONFIG_HPP_

#include "ipts.h"

#include <cstdint>
#include <string>

class IptsdConfig {
public:
	bool invert_x;
	bool invert_y;

	int32_t width;
	int32_t height;

	bool block_on_palm;
	int32_t touch_threshold;
	float stability_threshold;

	IptsdConfig(struct ipts_device_info info);

private:
	void load_dir(std::string name, struct ipts_device_info info);
};

#endif /* _IPTSD_CONFIG_HPP_ */
