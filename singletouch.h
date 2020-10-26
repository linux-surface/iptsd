/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_SINGLETOUCH_H_
#define _IPTSD_SINGLETOUCH_H_

#include "context.h"
#include "protocol.h"

int iptsd_singletouch_handle_input(struct iptsd_context *iptsd,
		struct ipts_data *header);

#endif /* _IPTSD_SINGLETOUCH_H_ */

