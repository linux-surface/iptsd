// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/uinput.h>
#include <stdint.h>
#include <string.h>

#include "context.h"
#include "devices.h"
#include "heatmap.h"
#include "protocol.h"
#include "touch.h"
#include "touch-processing.h"
#include "utils.h"

static void iptsd_touch_emit_empty(int dev)
{
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_POSITION_X, 0);
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_POSITION_Y, 0);
}

static void iptsd_touch_emit(int dev, struct iptsd_touch_input in)
{
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_TRACKING_ID, in.index);
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_POSITION_X, in.x);
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_POSITION_Y, in.y);
}

static int iptsd_touch_handle_heatmap(struct iptsd_context *iptsd,
		struct heatmap *hm)
{
	struct iptsd_touch_device touch = iptsd->devices.touch;
	bool blocked = false;

	iptsd_touch_processing_inputs(&touch.processor, hm);

	if (iptsd->config.block_on_palm) {
		for (int i = 0; i < touch.processor.max_contacts; i++)
			blocked = blocked || touch.processor.inputs[i].is_palm;
	}

	for (int i = 0; i < touch.processor.max_contacts; i++) {
		struct iptsd_touch_input in = touch.processor.inputs[i];

		iptsd_devices_emit(touch.dev, EV_ABS, ABS_MT_SLOT, in.slot);
		if (in.index != -1 && !in.is_stable)
			continue;

		if (in.is_palm || blocked) {
			iptsd_touch_emit_empty(touch.dev);
			continue;
		}

		iptsd_touch_emit(touch.dev, in);
	}

	int ret = iptsd_devices_emit(touch.dev, EV_SYN, SYN_REPORT, 0);
	if (ret < 0)
		iptsd_err(ret, "Failed to emit touch report");

	return ret;
}

int iptsd_touch_handle_input(struct iptsd_context *iptsd,
		struct ipts_payload_frame *frame)
{
	int pos = 0;
	struct heatmap *hm = NULL;

	while (pos < frame->size) {
		struct ipts_report *report =
			(struct ipts_report *)&frame->data[pos];

		uint8_t width = 0;
		uint8_t height = 0;

		switch (report->type) {
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP_DIM:
			height = report->data[0];
			width = report->data[1];

			hm = iptsd_touch_processing_get_heatmap(
					&iptsd->devices.touch.processor,
					width, height);
			break;
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP:
			memcpy(hm->data, report->data, hm->size);
			break;
		}

		pos += report->size + sizeof(struct ipts_report);
	}

	if (!hm)
		return 0;

	int ret = iptsd_touch_handle_heatmap(iptsd, hm);
	if (ret < 0)
		iptsd_err(ret, "Failed to handle touch heatmap");

	return ret;
}

