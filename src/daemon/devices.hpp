/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_DEVICES_HPP
#define IPTSD_DAEMON_DEVICES_HPP

#include "cone.hpp"
#include "config.hpp"
#include "touch-manager.hpp"
#include "uinput-device.hpp"

#include <common/types.hpp>

#include <memory>
#include <vector>

namespace iptsd::daemon {

class StylusDevice : public UinputDevice {
public:
	u32 serial;
	bool active = false;
	std::shared_ptr<Cone> cone;

	StylusDevice(Config conf, u32 serial, std::shared_ptr<Cone> cone);
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
	u32 active_styli = 0;

	DeviceManager(Config conf);

	StylusDevice &create_stylus(u32 serial);
	StylusDevice &get_stylus(u32 serial);
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_DEVICES_HPP */
