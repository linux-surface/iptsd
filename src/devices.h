/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DEVICES_H_
#define _IPTSD_DEVICES_H_

#include <stdbool.h>
#include <stdint.h>

#include "cone.h"
#include "config.h"
#include "constants.h"
#include "protocol.h"
#include "touch-processing.h"

struct iptsd_stylus_device {
	int dev;
	bool active;
	uint32_t serial;
	struct cone *cone;
};

struct iptsd_touch_device {
	int dev;
	struct iptsd_touch_processor processor;
};

struct iptsd_devices {
	struct iptsd_config config;
	struct ipts_device_info device_info;

	struct iptsd_touch_device touch;
	struct iptsd_stylus_device *active_stylus;
	struct iptsd_stylus_device styli[IPTSD_MAX_STYLI];
};

int iptsd_devices_add_stylus(struct iptsd_devices *devices, uint32_t serial);
int iptsd_devices_emit(int fd, int type, int code, int val);
int iptsd_devices_create(struct iptsd_devices *devices);
void iptsd_devices_destroy(struct iptsd_devices *devices);

#endif /* _IPTSD_DEVICES_H_ */
