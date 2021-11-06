// SPDX-License-Identifier: GPL-2.0-or-later

#include "touch.hpp"

#include "context.hpp"
#include "devices.hpp"
#include "touch-manager.hpp"

#include <ipts/parser.hpp>

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <vector>

namespace iptsd::daemon {

static void lift_mt(const TouchDevice &dev)
{
	dev.emit(EV_ABS, ABS_MT_TRACKING_ID, -1);
}

static void emit_mt(const TouchDevice &dev, const TouchInput &in)
{
	dev.emit(EV_ABS, ABS_MT_TRACKING_ID, in.index);
	dev.emit(EV_ABS, ABS_MT_POSITION_X, in.x);
	dev.emit(EV_ABS, ABS_MT_POSITION_Y, in.y);

	dev.emit(EV_ABS, ABS_MT_TOOL_TYPE, MT_TOOL_FINGER);
	dev.emit(EV_ABS, ABS_MT_TOOL_X, in.x);
	dev.emit(EV_ABS, ABS_MT_TOOL_Y, in.y);

	dev.emit(EV_ABS, ABS_MT_ORIENTATION, in.orientation);
	dev.emit(EV_ABS, ABS_MT_TOUCH_MAJOR, in.major);
	dev.emit(EV_ABS, ABS_MT_TOUCH_MINOR, in.minor);
}

static void lift_st(const TouchDevice &dev)
{
	dev.emit(EV_KEY, BTN_TOUCH, 0);
}

static void emit_st(const TouchDevice &dev, const TouchInput &in)
{
	dev.emit(EV_KEY, BTN_TOUCH, 1);
	dev.emit(EV_ABS, ABS_X, in.x);
	dev.emit(EV_ABS, ABS_Y, in.y);
}

static void handle_single(const TouchDevice &touch, const std::vector<TouchInput> &inputs)
{
	for (const TouchInput &in : inputs) {
		if (in.active && !in.stable)
			return;

		if (!in.active || in.palm)
			continue;

		emit_st(touch, in);
		return;
	}

	lift_st(touch);
}

static void handle_multi(const TouchDevice &touch, const std::vector<TouchInput> &inputs)
{
	for (const TouchInput &in : inputs) {
		touch.emit(EV_ABS, ABS_MT_SLOT, in.index);

		if (in.active && !in.stable)
			continue;

		if (!in.active || in.palm) {
			lift_mt(touch);
			continue;
		}

		emit_mt(touch, in);
	}
}

void iptsd_touch_input(Context &ctx, const ipts::Heatmap &data)
{
	TouchDevice &touch = ctx.devices.touch;

	// Dont process any touches and lift existing ones if a stylus is in proximity
	if (ctx.devices.active_styli > 0 && ctx.config.disable_touch_on_stylus) {
		lift_st(touch);

		for (u8 i = 0; i < ctx.control.info.max_contacts; i++) {
			touch.emit(EV_ABS, ABS_MT_SLOT, i);
			lift_mt(touch);
		}
	} else {
		const std::vector<TouchInput> &inputs = touch.manager.process(data);

		handle_multi(touch, inputs);
		handle_single(touch, inputs);
	}

	touch.emit(EV_SYN, SYN_REPORT, 0);
}

} // namespace iptsd::daemon
