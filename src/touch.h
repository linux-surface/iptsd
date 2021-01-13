/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_TOUCH_H_
#define _IPTSD_TOUCH_H_

#include "context.h"
#include "protocol.h"

int iptsd_touch_handle_input(struct iptsd_context *iptsd, struct ipts_payload_frame frame);

#endif /* _IPTSD_TOUCH_H_ */
