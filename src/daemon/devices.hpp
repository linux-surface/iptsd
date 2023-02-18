/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_DEVICES_HPP
#define IPTSD_DAEMON_DEVICES_HPP

#include "cone.hpp"
#include "uinput-device.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/finder.hpp>

#include <memory>
#include <vector>

namespace iptsd::daemon {

class StylusDevice : public UinputDevice {
public:
	bool active = false;
	std::shared_ptr<Cone> cone;

public:
	StylusDevice(const config::Config &conf, std::shared_ptr<Cone> cone);
};

class TouchDevice : public UinputDevice {
public:
	std::shared_ptr<Cone> cone;
	contacts::ContactFinder finder;

public:
	TouchDevice(const config::Config &conf, std::shared_ptr<Cone> cone);
};

class DeviceManager {
public:
	std::unique_ptr<TouchDevice> touch;
	std::unique_ptr<StylusDevice> stylus;

public:
	DeviceManager(const config::Config &conf);
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_DEVICES_HPP */
