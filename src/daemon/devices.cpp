// SPDX-License-Identifier: GPL-2.0-or-later

#include "devices.hpp"

#include "uinput-device.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/finder.hpp>
#include <ipts/protocol.hpp>

#include <climits>
#include <cmath>
#include <cstddef>
#include <gsl/gsl>
#include <iterator>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace iptsd::daemon {

DeviceManager::DeviceManager(const config::Config &conf) : touch {nullptr}, stylus {nullptr}
{
	auto cone = std::make_shared<Cone>(conf.cone_angle, conf.cone_distance);

	this->touch = std::make_unique<TouchDevice>(conf, cone);
	this->stylus = std::make_unique<StylusDevice>(conf, cone);
}

} // namespace iptsd::daemon
