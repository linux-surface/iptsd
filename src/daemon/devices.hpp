/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DAEMON_DEVICES_HPP_
#define _IPTSD_DAEMON_DEVICES_HPP_

#include "config.hpp"
#include "touch-manager.hpp"
#include "uinput-device.hpp"

#include <common/types.hpp>

#include <vector>

class StylusDevice : public UinputDevice {
public:
	u32 serial = 0;

	StylusDevice(IptsdConfig conf);
};

class TouchDevice : public UinputDevice {
public:
	TouchManager manager;

	TouchDevice(IptsdConfig conf);
};

class DeviceManager {
public:
	IptsdConfig conf;

	TouchDevice touch;
	std::vector<StylusDevice> styli;

	size_t active = 0;

	DeviceManager(IptsdConfig conf);

	void switch_stylus(u32 serial);
	StylusDevice &active_stylus();
};

#endif /* _IPTSD_DAEMON_DEVICES_HPP_ */
