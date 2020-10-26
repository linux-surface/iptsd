/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONTEXT_H_
#define _IPTSD_CONTEXT_H_

#include "config.h"
#include "control.h"
#include "devices.h"

struct iptsd_context {
	struct iptsd_control control;
	struct iptsd_devices devices;
	struct iptsd_config config;
	char *data;
};

#endif /* _IPTSD_CONTEXT_H_ */

