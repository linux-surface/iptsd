// SPDX-License-Identifier: GPL-2.0-or-later

#include "devices.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/finder.hpp>
#include <ipts/protocol.hpp>

#include <climits>
#include <cmath>
#include <cstddef>
#include <gsl/gsl>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace iptsd::daemon {

DeviceManager::DeviceManager(const config::Config &conf) : conf {conf}, touch {conf}
{
	this->create_stylus(0);
}

StylusDevice &DeviceManager::create_stylus(u32 serial)
{
	std::shared_ptr<Cone> cone = std::make_shared<Cone>(conf.cone_angle, conf.cone_distance);
	this->touch.cones.push_back(cone);
	return this->styli.emplace_back(serial, std::move(cone));
}

StylusDevice &DeviceManager::get_stylus(u32 serial)
{
	StylusDevice &stylus = this->styli.back();

	if (stylus.serial == serial)
		return stylus;

	if (stylus.serial == 0) {
		stylus.serial = serial;
		return stylus;
	}

	for (StylusDevice &s : this->styli) {
		if (s.serial != serial)
			continue;

		std::swap(s, stylus);
		return s;
	}

	return this->create_stylus(serial);
}

} // namespace iptsd::daemon
