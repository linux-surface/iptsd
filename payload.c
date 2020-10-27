// SPDX-License-Identifier: GPL-2.0-or-later

#include "context.h"
#include "protocol.h"
#include "stylus.h"
#include "touch.h"
#include "syscall.h"

int iptsd_payload_handle_input(struct iptsd_context *iptsd,
		struct ipts_data *header)
{
	int pos = 0;
	struct ipts_payload *payload = (struct ipts_payload *)header->data;

	for (int i = 0; i < payload->frames; i++) {
		int ret = 0;
		struct ipts_payload_frame *frame =
			(struct ipts_payload_frame *)&payload->data[pos];

		switch (frame->type) {
		case IPTS_PAYLOAD_FRAME_TYPE_STYLUS:
			ret = iptsd_stylus_handle_input(iptsd, frame);
			break;
		case IPTS_PAYLOAD_FRAME_TYPE_TOUCH:
			ret = iptsd_touch_handle_input(iptsd, frame);
			break;
		}

		if (ret < 0) {
			iptsd_err(ret, "Failed to handle payload frame");
			return ret;
		}

		pos = pos + sizeof(struct ipts_payload_frame) + frame->size;
	}

	return 0;
}

