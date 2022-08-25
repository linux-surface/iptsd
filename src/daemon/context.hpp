/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_CONTEXT_HPP
#define IPTSD_DAEMON_CONTEXT_HPP

#include "devices.hpp"

#include <common/types.hpp>
#include <config/config.hpp>

namespace iptsd::daemon {

class Context {
public:
	config::Config config;
	DeviceManager devices;

public:
	Context(i16 vendor, i16 product) : config {vendor, product}, devices {config} {};
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_CONTEXT_HPP */
