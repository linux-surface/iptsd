/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DAEMON_CONTEXT_HPP_
#define _IPTSD_DAEMON_CONTEXT_HPP_

#include "config.hpp"
#include "devices.hpp"

#include <ipts/control.hpp>
#include <ipts/parser.hpp>

#include <utility>

class IptsdContext {
public:
	IptsdConfig *config;
	IptsControl *control;
	IptsParser *parser;
	DeviceManager *devices;

	~IptsdContext(void)
	{
		delete std::exchange(this->config, nullptr);
		delete std::exchange(this->control, nullptr);
		delete std::exchange(this->devices, nullptr);
	};
};

#endif /* _IPTSD_DAEMON_CONTEXT_HPP_ */
