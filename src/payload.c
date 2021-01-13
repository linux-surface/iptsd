// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdint.h>

#include "context.h"
#include "payload.h"
#include "protocol.h"
#include "reader.h"
#include "stylus.h"
#include "touch.h"
#include "utils.h"

int iptsd_payload_handle_input(struct iptsd_context *iptsd)
{
	struct ipts_payload payload;

	int ret = iptsd_reader_read(&iptsd->reader, &payload, sizeof(struct ipts_payload));
	if (ret < 0) {
		iptsd_err(ret, "Received invalid data");
		return 0;
	}

	for (uint32_t i = 0; i < payload.frames; i++) {
		struct ipts_payload_frame frame;

		ret = iptsd_reader_read(&iptsd->reader, &frame, sizeof(struct ipts_payload_frame));
		if (ret < 0) {
			iptsd_err(ret, "Received invalid data");
			return 0;
		}

		switch (frame.type) {
		case IPTS_PAYLOAD_FRAME_TYPE_STYLUS:
			ret = iptsd_stylus_handle_input(iptsd, frame);
			break;
		case IPTS_PAYLOAD_FRAME_TYPE_TOUCH:
			ret = iptsd_touch_handle_input(iptsd, frame);
			break;
		default:
			iptsd_reader_skip(&iptsd->reader, frame.size);
			break;
		}

		if (ret < 0) {
			iptsd_err(ret, "Failed to handle payload frame");
			return ret;
		}
	}

	return 0;
}
