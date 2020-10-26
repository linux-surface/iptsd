// SPDX-License-Identifier: GPL-2.0-or-later

#include "context.h"
#include "payload.h"
#include "singletouch.h"

int iptsd_hid_handle_input(struct iptsd_context *iptsd,
		struct ipts_data *header)
{
	/*
	 * Make sure that we only handle singletouch inputs.
	 */
	if (header->data[0] != IPTS_SINGLETOUCH_REPORT_ID)
		return 0;

	return iptsd_singletouch_handle_input(iptsd, header);
}
