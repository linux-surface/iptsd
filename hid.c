// SPDX-License-Identifier: GPL-2.0-or-later

#include "context.h"
#include "payload.h"
#include "singletouch.h"
#include "syscall.h"

int iptsd_hid_handle_input(struct iptsd_context *iptsd,
		struct ipts_data *header)
{
	/*
	 * Make sure that we only handle singletouch inputs.
	 */
	if (header->data[0] != IPTS_SINGLETOUCH_REPORT_ID)
		return 0;

	int ret = iptsd_singletouch_handle_input(iptsd, header);
	if (ret < 0)
		iptsd_err(ret, "Failed to handle singletouch input");

	return ret;
}

