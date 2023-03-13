/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_DEVICES_HPP
#define IPTSD_DAEMON_DEVICES_HPP

#include "cone.hpp"
#include "contacts/contact.hpp"
#include "uinput-device.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/finder.hpp>

#include <list>
#include <memory>
#include <set>
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
	Image<f32> heatmap {};

	contacts::Finder<f32, f64> finder;
	std::vector<contacts::Contact<f32>> contacts {};

	std::set<usize> current {};
	std::set<usize> last {};
	std::set<usize> lift {};

	std::shared_ptr<Cone> cone;

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
