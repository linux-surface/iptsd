/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DEVICES_H_
#define _IPTSD_DEVICES_H_

#include <stdbool.h>
#include <stdint.h>

#include "stylus-processing.h"
#include "touch-processing.h"

#define IPTSD_MAX_STYLI 10

struct iptsd_device_config {
	int width;
	int height;
	bool invert_x;
	bool invert_y;

	uint16_t vendor;
	uint16_t product;
	uint32_t version;
	uint32_t max_contacts;
};

struct iptsd_stylus_device {
	int dev;
	bool active;
	uint32_t serial;
	struct iptsd_stylus_processor processor;
};

struct iptsd_touch_device {
	int dev;
	struct iptsd_touch_processor processor;
};

struct iptsd_devices {
	struct iptsd_device_config config;
	struct iptsd_touch_device touch;
	struct iptsd_stylus_device *active_stylus;
	struct iptsd_stylus_device styli[IPTSD_MAX_STYLI];
};

int iptsd_devices_add_stylus(struct iptsd_devices *devices, uint32_t serial);
int iptsd_devices_emit(int fd, int type, int code, int val);
int iptsd_devices_create(struct iptsd_devices *devices);
void iptsd_devices_destroy(struct iptsd_devices *devices);

#endif /* _IPTSD_DEVICES_H_ */

