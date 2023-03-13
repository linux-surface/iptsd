/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_DEVICES_HPP
#define IPTSD_DAEMON_DEVICES_HPP

#include "cone.hpp"
#include "stylus.hpp"
#include "touch.hpp"
#include "uinput-device.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/finder.hpp>

#include <list>
#include <memory>
#include <set>
#include <vector>

namespace iptsd::daemon {

class DeviceManager {
public:
	std::unique_ptr<TouchDevice> touch;
	std::unique_ptr<StylusDevice> stylus;

public:
	DeviceManager(const config::Config &conf);
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_DEVICES_HPP */
