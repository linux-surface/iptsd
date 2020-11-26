// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/input-event-codes.h>
#include <linux/uinput.h>

#include "context.h"
#include "devices.h"
#include "protocol.h"
#include "singletouch.h"
#include "utils.h"

static void iptsd_singletouch_lift(int dev)
{
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_SLOT, 0);
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_TRACKING_ID, -1);

	iptsd_devices_emit(dev, EV_KEY, BTN_TOUCH, 0);
}

static void iptsd_singletouch_emit(int dev, struct ipts_singletouch_data *data)
{
	int x = (int)((double)data->x / 32767 * IPTS_MAX_X);
	int y = (int)((double)data->y / 32767 * IPTS_MAX_Y);

	iptsd_devices_emit(dev, EV_ABS, ABS_MT_SLOT, 0);

	iptsd_devices_emit(dev, EV_ABS, ABS_MT_TRACKING_ID, 0);
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_POSITION_X, x);
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_POSITION_Y, y);

	iptsd_devices_emit(dev, EV_ABS, ABS_MT_TOOL_TYPE, MT_TOOL_FINGER);
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_TOOL_X, x);
	iptsd_devices_emit(dev, EV_ABS, ABS_MT_TOOL_Y, y);

	iptsd_devices_emit(dev, EV_KEY, BTN_TOUCH, 1);
	iptsd_devices_emit(dev, EV_ABS, ABS_X, x);
	iptsd_devices_emit(dev, EV_ABS, ABS_Y, y);
}


int iptsd_singletouch_handle_input(struct iptsd_context *iptsd,
		struct ipts_data *header)
{
	struct iptsd_touch_device touch = iptsd->devices.touch;

	struct ipts_singletouch_data *data =
		(struct ipts_singletouch_data *)&header->data[1];

	if (data->touch)
		iptsd_singletouch_emit(touch.dev, data);
	else
		iptsd_singletouch_lift(touch.dev);

	int ret = iptsd_devices_emit(touch.dev, EV_SYN, SYN_REPORT, 0);
	if (ret < 0)
		iptsd_err(ret, "Failed to emit singletouch report");

	return ret;
}

