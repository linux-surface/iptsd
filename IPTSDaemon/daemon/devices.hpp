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
	bool active = false;
	std::shared_ptr<Cone> cone;

public:
    StylusDevice(std::shared_ptr<Cone> cone): cone{std::move(cone)} {}
};

class TouchDevice {
public:
	std::shared_ptr<Cone> cone;
	contacts::ContactFinder finder;

public:
    TouchDevice(const config::Config &conf, std::shared_ptr<Cone> cone): finder{conf.contacts()}, cone{std::move(cone)} {}
};

class DeviceManager {
public:
	std::unique_ptr<TouchDevice> touch;
	std::unique_ptr<StylusDevice> stylus;

public:
	DeviceManager(const config::Config &conf) {
        auto cone = std::make_shared<Cone>(conf.cone_angle, conf.cone_distance);

        this->touch = std::make_unique<TouchDevice>(conf, cone);
        this->stylus = std::make_unique<StylusDevice>(conf, cone);
    }
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_DEVICES_HPP */
