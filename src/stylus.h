/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_STYLUS_H_
#define _IPTSD_STYLUS_H_

#include "context.h"
#include "protocol.h"

int iptsd_stylus_handle_input(struct iptsd_context *iptsd, struct ipts_payload_frame frame);

#endif /* _IPTSD_STYLUS_H_ */
