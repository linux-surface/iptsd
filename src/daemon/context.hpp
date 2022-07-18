/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_CONTEXT_HPP
#define IPTSD_DAEMON_CONTEXT_HPP

#include "config.hpp"
#include "devices.hpp"

#include <common/types.hpp>

namespace iptsd::daemon {

class Context {
public:
	Config config;
	DeviceManager devices;

	Context(i16 vendor, i16 product) : config(vendor, product), devices(config) {};
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_CONTEXT_HPP */
