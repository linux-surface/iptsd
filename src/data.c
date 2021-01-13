// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdio.h>

#include "context.h"
#include "data.h"
#include "hid.h"
#include "payload.h"
#include "protocol.h"
#include "reader.h"
#include "utils.h"

int iptsd_data_handle_input(struct iptsd_context *iptsd)
{
	struct ipts_data header;

	int ret = iptsd_reader_read(&iptsd->reader, &header, sizeof(struct ipts_data));
	if (ret < 0) {
		iptsd_err(ret, "Received invalid data");
		return 0;
	}

	switch (header.type) {
	case IPTS_DATA_TYPE_PAYLOAD:
		ret = iptsd_payload_handle_input(iptsd);
		break;
	case IPTS_DATA_TYPE_HID_REPORT:
		ret = iptsd_hid_handle_input(iptsd);
		break;
	default:
		iptsd_reader_skip(&iptsd->reader, header.size);
		break;
	}

	if (ret < 0)
		iptsd_err(ret, "Failed to parse data");

	return ret;
}
