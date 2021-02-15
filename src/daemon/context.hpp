/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONTEXT_HPP_
#define _IPTSD_CONTEXT_HPP_

#include "config.hpp"
#include "control.hpp"
#include "devices.hpp"
#include "reader.hpp"

class IptsdContext {
public:
	IptsdConfig *config;
	IptsdControl *control;
	IptsdReader *reader;
	DeviceManager *devices;

	~IptsdContext(void)
	{
		delete this->config;
		delete this->control;
		delete this->reader;
		delete this->devices;
	};
};

#endif /* _IPTSD_CONTEXT_HPP_ */
