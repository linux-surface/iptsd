// SPDX-License-Identifier: GPL-2.0-or-later

#include "singletouch.hpp"

#include "context.hpp"
#include "devices.hpp"
#include "protocol.h"
#include "reader.hpp"
#include "types.hpp"

#include <linux/input-event-codes.h>
#include <linux/input.h>

static void lift(TouchDevice dev)
{
	dev.emit(EV_ABS, ABS_MT_SLOT, 0);
	dev.emit(EV_ABS, ABS_MT_TRACKING_ID, -1);
	dev.emit(EV_KEY, BTN_TOUCH, 0);
}

static void emit(TouchDevice dev, struct ipts_singletouch_data data)
{
	f64 rX = (f64)data.x / IPTS_SINGLETOUCH_MAX_VALUE;
	f64 rY = (f64)data.y / IPTS_SINGLETOUCH_MAX_VALUE;

	i32 x = (i32)(rX * IPTS_MAX_X);
	i32 y = (i32)(rY * IPTS_MAX_Y);

	dev.emit(EV_ABS, ABS_MT_SLOT, 0);

	dev.emit(EV_ABS, ABS_MT_TRACKING_ID, 0);
	dev.emit(EV_ABS, ABS_MT_POSITION_X, x);
	dev.emit(EV_ABS, ABS_MT_POSITION_Y, y);

	dev.emit(EV_ABS, ABS_MT_TOOL_TYPE, MT_TOOL_FINGER);
	dev.emit(EV_ABS, ABS_MT_TOOL_X, x);
	dev.emit(EV_ABS, ABS_MT_TOOL_Y, y);

	dev.emit(EV_KEY, BTN_TOUCH, 1);
	dev.emit(EV_ABS, ABS_X, x);
	dev.emit(EV_ABS, ABS_Y, y);
}

void iptsd_singletouch_handle_input(IptsdContext *iptsd)
{
	auto data = iptsd->reader->read<struct ipts_singletouch_data>();

	if (data.touch)
		emit(iptsd->devices->touch, data);
	else
		lift(iptsd->devices->touch);

	iptsd->devices->touch.emit(EV_SYN, SYN_REPORT, 0);
}
