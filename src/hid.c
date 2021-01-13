// SPDX-License-Identifier: GPL-2.0-or-later

#include "hid.h"
#include "context.h"
#include "payload.h"
#include "reader.h"
#include "singletouch.h"
#include "utils.h"

int iptsd_hid_handle_input(struct iptsd_context *iptsd)
{
	uint8_t report;

	int ret = iptsd_reader_read(&iptsd->reader, &report, sizeof(uint8_t));
	if (ret < 0) {
		iptsd_err(ret, "Received invalid data");
		return 0;
	}

	/*
	 * Make sure that we only handle singletouch inputs.
	 */
	if (report != IPTS_SINGLETOUCH_REPORT_ID)
		return 0;

	ret = iptsd_singletouch_handle_input(iptsd);
	if (ret < 0)
		iptsd_err(ret, "Failed to handle singletouch input");

	return ret;
}
