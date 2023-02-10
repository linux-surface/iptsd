/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_DEVICES_HPP
#define IPTSD_DAEMON_DEVICES_HPP

#include "cone.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/finder.hpp>

#include <memory>
#include <vector>

namespace iptsd::daemon {

class StylusDevice {
public:
	u32 serial;
	bool active = false;
	std::shared_ptr<Cone> cone;

public:
    StylusDevice(u32 serial, std::shared_ptr<Cone> cone): serial(serial), cone(std::move(cone)) {};
};

class TouchDevice {
public:
	std::vector<std::shared_ptr<Cone>> cones;
	contacts::ContactFinder finder;

public:
    TouchDevice(const config::Config &conf): finder(conf.contacts()) {};
};

class DeviceManager {
public:
	const config::Config &conf;
	TouchDevice touch;

	std::vector<StylusDevice> styli;
	u32 active_styli = 0;

public:
	DeviceManager(const config::Config &conf);

	StylusDevice &create_stylus(u32 serial);
	StylusDevice &get_stylus(u32 serial);
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_DEVICES_HPP */
