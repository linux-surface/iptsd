// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/uinput.h>

#include "context.h"
#include "devices.h"
#include "protocol.h"
#include "singletouch.h"
#include "utils.h"

int iptsd_singletouch_handle_input(struct iptsd_context *iptsd,
		struct ipts_data *header)
{
	struct iptsd_touch_device touch = iptsd->devices.touch;

	struct ipts_singletouch_data *data =
		(struct ipts_singletouch_data *)&header->data[1];

	int x = (int)((double)data->x / 32767 * IPTS_MAX_X);
	int y = (int)((double)data->y / 32767 * IPTS_MAX_Y);

	iptsd_devices_emit(touch.dev, EV_ABS, ABS_MT_SLOT, 0);

	if (data->touch) {
		iptsd_devices_emit(touch.dev, EV_ABS, ABS_MT_TRACKING_ID, 0);
		iptsd_devices_emit(touch.dev, EV_ABS, ABS_MT_POSITION_X, x);
		iptsd_devices_emit(touch.dev, EV_ABS, ABS_MT_POSITION_Y, y);
	} else {
		iptsd_devices_emit(touch.dev, EV_ABS, ABS_MT_TRACKING_ID, -1);
	}

	int ret = iptsd_devices_emit(touch.dev, EV_SYN, SYN_REPORT, 0);
	if (ret < 0)
		iptsd_err(ret, "Failed to emit singletouch report");

	return ret;
}

