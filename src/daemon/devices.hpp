/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_DEVICES_HPP
#define IPTSD_DAEMON_DEVICES_HPP

#include "cone.hpp"
#include "config.hpp"
#include "uinput-device.hpp"

#include <common/types.hpp>
#include <contacts/finder.hpp>
#include <drm/device.hpp>

#include <memory>
#include <vector>

namespace iptsd::daemon {

class StylusDevice : public UinputDevice {
public:
	u32 serial;
	bool active = false;
	std::shared_ptr<Cone> cone;

public:
	StylusDevice(const Config &conf, const drm::Device &display, u32 serial,
		     std::shared_ptr<Cone> cone);
};

class TouchDevice : public UinputDevice {
public:
	std::vector<std::shared_ptr<Cone>> cones;
	contacts::ContactFinder finder;

public:
	TouchDevice(const Config &conf, const drm::Device &display);
};

class DeviceManager {
public:
	const Config &conf;
	const drm::Device &display;

	TouchDevice touch;

	std::vector<StylusDevice> styli;
	u32 active_styli = 0;

public:
	DeviceManager(const Config &conf, const drm::Device &display);

	StylusDevice &create_stylus(u32 serial);
	StylusDevice &get_stylus(u32 serial);
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_DEVICES_HPP */
