/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_CONTEXT_HPP
#define IPTSD_DAEMON_CONTEXT_HPP

#include "config.hpp"
#include "devices.hpp"

#include <common/types.hpp>
#include <drm/device.hpp>

namespace iptsd::daemon {

class Context {
public:
	Config config;
	drm::Device display;
	DeviceManager devices;

public:
	Context(i16 vendor, i16 product)
		: config {vendor, product}, display {}, devices {config, display} {};
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_CONTEXT_HPP */
