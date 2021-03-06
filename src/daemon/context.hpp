/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_CONTEXT_HPP
#define IPTSD_DAEMON_CONTEXT_HPP

#include "config.hpp"
#include "devices.hpp"

#include <ipts/control.hpp>
#include <ipts/parser.hpp>

#include <utility>

class IptsdContext {
public:
	IptsControl control;
	IptsdConfig config;
	DeviceManager devices;
	IptsParser parser;

	IptsdContext()
		: control(), config(control.info), devices(config),
		  parser(control.info.buffer_size) {};
};

#endif /* IPTSD_DAEMON_CONTEXT_HPP */
