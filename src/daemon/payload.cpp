// SPDX-License-Identifier: GPL-2.0-or-later

#include "payload.hpp"

#include "context.hpp"
#include "protocol.h"
#include "reader.hpp"
#include "stylus.hpp"
#include "touch.hpp"
#include "types.hpp"

void iptsd_payload_handle_input(IptsdContext *iptsd)
{
	auto payload = iptsd->reader->read<struct ipts_payload>();

	for (u32 i = 0; i < payload.frames; i++) {
		auto frame = iptsd->reader->read<struct ipts_payload_frame>();

		switch (frame.type) {
		case IPTS_PAYLOAD_FRAME_TYPE_STYLUS:
			iptsd_stylus_handle_input(iptsd, frame);
			break;
		case IPTS_PAYLOAD_FRAME_TYPE_TOUCH:
			iptsd_touch_handle_input(iptsd, frame);
			break;
		default:
			iptsd->reader->skip(frame.size);
			break;
		}
	}
}
