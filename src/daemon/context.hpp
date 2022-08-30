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
	Context(const config::Config &config) : config {config}, devices {config} {};
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_CONTEXT_HPP */
