// SPDX-License-Identifier: GPL-2.0-or-later

#include "hid.hpp"

#include "context.hpp"
#include "protocol.h"
#include "reader.hpp"
#include "singletouch.hpp"

#include <cstdint>

void iptsd_hid_handle_input(IptsdContext *iptsd)
{
	auto report = iptsd->reader->read<uint8_t>();

	/*
	 * Make sure that we only handle singletouch inputs.
	 */
	if (report != IPTS_SINGLETOUCH_REPORT_ID)
		return;

	iptsd_singletouch_handle_input(iptsd);
}
