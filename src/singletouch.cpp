// SPDX-License-Identifier: GPL-2.0-or-later

#include "singletouch.hpp"

#include "context.hpp"
#include "devices.hpp"
#include "protocol.h"
#include "reader.hpp"

#include <cstdint>
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
	double rX = (double)data.x / IPTS_SINGLETOUCH_MAX_VALUE;
	double rY = (double)data.y / IPTS_SINGLETOUCH_MAX_VALUE;

	int32_t x = (int32_t)(rX * IPTS_MAX_X);
	int32_t y = (int32_t)(rY * IPTS_MAX_Y);

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
