/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONFIG_H_
#define _IPTSD_CONFIG_H_

#include <stdbool.h>

#include "ipts.h"

struct iptsd_config {
	bool invert_x;
	bool invert_y;

	int width;
	int height;

	bool block_on_palm;
	int touch_threshold;
	float stability_threshold;
};

void iptsd_config_load(struct iptsd_config *config, struct ipts_device_info info);

#endif /* _IPTSD_CONFIG_H_ */
