/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_HID_H_
#define _IPTSD_HID_H_

#include "context.h"
#include "protocol.h"

int iptsd_hid_handle_input(struct iptsd_context *iptsd,
		struct ipts_data *header);

#endif /* _IPTSD_HID_H_ */

