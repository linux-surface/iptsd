/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DEVICES_HPP_
#define _IPTSD_DEVICES_HPP_

#include "cone.hpp"
#include "config.hpp"
#include "heatmap.hpp"
#include "ipts.h"
#include "touch-processing.hpp"
#include "uinput-device.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

class StylusDevice : public UinputDevice {
public:
	Cone cone;
	uint32_t serial;

	StylusDevice(struct ipts_device_info info, IptsdConfig *conf);
};

class TouchDevice : public UinputDevice {
public:
	Heatmap *hm;
	TouchProcessor processor;
	IptsdConfig *conf;

	TouchDevice(struct ipts_device_info, IptsdConfig *conf);

	Heatmap *get_heatmap(int32_t w, int32_t h);
};

class DeviceManager {
public:
	IptsdConfig *conf;
	TouchDevice touch;
	size_t active_index;
	struct ipts_device_info info;
	std::vector<StylusDevice *> styli;

	DeviceManager(struct ipts_device_info info, IptsdConfig *conf);
	~DeviceManager(void);

	StylusDevice *active_stylus(void);
	void switch_stylus(uint32_t serial);
};

#endif /* _IPTSD_DEVICES_HPP_ */
