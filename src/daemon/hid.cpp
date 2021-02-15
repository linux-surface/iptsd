// SPDX-License-Identifier: GPL-2.0-or-later

#include "hid.hpp"

#include "context.hpp"
#include "reader.hpp"
#include "singletouch.hpp"

#include <common/types.hpp>
#include <ipts/protocol.h>

void iptsd_hid_handle_input(IptsdContext *iptsd)
{
	auto report = iptsd->reader->read<u8>();

	/*
	 * Make sure that we only handle singletouch inputs.
	 */
	if (report != IPTS_SINGLETOUCH_REPORT_ID)
		return;

	iptsd_singletouch_handle_input(iptsd);
}
