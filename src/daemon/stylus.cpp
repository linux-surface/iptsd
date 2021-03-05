// SPDX-License-Identifier: GPL-2.0-or-later

#include "stylus.hpp"

#include "config.hpp"
#include "context.hpp"
#include "devices.hpp"

#include <common/types.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.h>

#include <cmath>
#include <linux/input-event-codes.h>
#include <tuple>

static std::tuple<i32, i32> get_tilt(u32 altitude, u32 azimuth)
{
	if (altitude <= 0)
		return std::tuple<i32, i32>(0, 0);

	f32 alt = ((f32)altitude) / 18000 * M_PI;
	f32 azm = ((f32)azimuth) / 18000 * M_PI;

	f32 sin_alt = std::sin(alt);
	f32 sin_azm = std::sin(azm);

	f32 cos_alt = std::cos(alt);
	f32 cos_azm = std::cos(azm);

	f32 atan_x = std::atan2(cos_alt, sin_alt * cos_azm);
	f32 atan_y = std::atan2(cos_alt, sin_alt * sin_azm);

	i32 tx = 9000 - (atan_x * 4500 / M_PI_4);
	i32 ty = (atan_y * 4500 / M_PI_4) - 9000;

	return std::tuple<i32, i32>(tx, ty);
}

void iptsd_stylus_input(IptsdContext *iptsd, IptsStylusData data)
{
	StylusDevice *stylus = iptsd->devices.active_stylus;

	bool prox = (data.mode & IPTS_STYLUS_REPORT_MODE_PROX) >> 0;
	bool touch = (data.mode & IPTS_STYLUS_REPORT_MODE_TOUCH) >> 1;
	bool button = (data.mode & IPTS_STYLUS_REPORT_MODE_BUTTON) >> 2;
	bool rubber = (data.mode & IPTS_STYLUS_REPORT_MODE_RUBBER) >> 3;

	bool btn_pen = prox && !rubber;
	bool btn_rubber = prox && rubber;

	std::tuple<i32, i32> tilt = get_tilt(data.altitude, data.azimuth);

	stylus->emit(EV_KEY, BTN_TOUCH, touch);
	stylus->emit(EV_KEY, BTN_TOOL_PEN, btn_pen);
	stylus->emit(EV_KEY, BTN_TOOL_RUBBER, btn_rubber);
	stylus->emit(EV_KEY, BTN_STYLUS, button);

	stylus->emit(EV_ABS, ABS_X, data.x);
	stylus->emit(EV_ABS, ABS_Y, data.y);
	stylus->emit(EV_ABS, ABS_PRESSURE, data.pressure);
	stylus->emit(EV_ABS, ABS_MISC, data.timestamp);

	stylus->emit(EV_ABS, ABS_TILT_X, std::get<0>(tilt));
	stylus->emit(EV_ABS, ABS_TILT_Y, std::get<1>(tilt));

	stylus->emit(EV_SYN, SYN_REPORT, 0);
}
