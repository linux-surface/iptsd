/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DAEMON_CONTEXT_HPP_
#define _IPTSD_DAEMON_CONTEXT_HPP_

#include "config.hpp"
#include "devices.hpp"

#include <ipts/control.hpp>
#include <ipts/parser.hpp>

class IptsdContext {
public:
	IptsdConfig *config;
	IptsdControl *control;
	IptsParser *parser;
	DeviceManager *devices;

	~IptsdContext(void)
	{
		delete this->config;
		delete this->control;
		delete this->devices;
	};
};

#endif /* _IPTSD_DAEMON_CONTEXT_HPP_ */
