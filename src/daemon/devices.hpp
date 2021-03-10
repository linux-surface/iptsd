/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_DEVICES_HPP
#define IPTSD_DAEMON_DEVICES_HPP

#include "config.hpp"
#include "touch-manager.hpp"
#include "uinput-device.hpp"

#include <common/types.hpp>

#include <vector>

namespace iptsd::daemon {

class StylusDevice : public UinputDevice {
public:
	u32 serial;

	StylusDevice(Config conf, u32 serial);
};

class TouchDevice : public UinputDevice {
public:
	TouchManager manager;

	TouchDevice(Config conf);
};

class DeviceManager {
public:
	Config conf;
	TouchDevice touch;
	std::vector<StylusDevice> styli;

	DeviceManager(Config conf);

	StylusDevice &get_stylus(u32 serial);
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_DEVICES_HPP */
