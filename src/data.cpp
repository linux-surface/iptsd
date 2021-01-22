// SPDX-License-Identifier: GPL-2.0-or-later

#include "data.hpp"

#include "context.hpp"
#include "hid.hpp"
#include "payload.hpp"
#include "protocol.h"
#include "reader.hpp"

void iptsd_data_handle_input(IptsdContext *iptsd)
{
	auto header = iptsd->reader->read<struct ipts_data>();

	switch (header.type) {
	case IPTS_DATA_TYPE_PAYLOAD:
		iptsd_payload_handle_input(iptsd);
		break;
	case IPTS_DATA_TYPE_HID_REPORT:
		iptsd_hid_handle_input(iptsd);
		break;
	default:
		iptsd->reader->skip(header.size);
		break;
	}
}
