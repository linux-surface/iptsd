/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_CONTEXT_HPP
#define IPTSD_DAEMON_CONTEXT_HPP

#include "config.hpp"
#include "devices.hpp"

#include <ipts/control.hpp>
#include <ipts/parser.hpp>

#include <utility>

namespace iptsd::daemon {

class Context {
public:
	ipts::Control control;
	ipts::Parser parser;

	Config config;
	DeviceManager devices;

	Context()
		: control(), parser(control.info.buffer_size), config(control.info),
		  devices(config) {};
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_CONTEXT_HPP */
