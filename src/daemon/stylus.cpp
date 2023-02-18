// SPDX-License-Identifier: GPL-2.0-or-later

#include "stylus.hpp"

#include "context.hpp"
#include "devices.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.hpp>

#include <cmath>
#include <gsl/gsl>
#include <linux/input-event-codes.h>
#include <tuple>

namespace iptsd::daemon {

static std::tuple<i32, i32> get_tilt(u32 altitude, u32 azimuth)
{
	if (altitude <= 0)
		return {0, 0};

	f64 alt = static_cast<f64>(altitude) / 18000 * math::num<f64>::pi;
	f64 azm = static_cast<f64>(azimuth) / 18000 * math::num<f64>::pi;

	f64 sin_alt = std::sin(alt);
	f64 sin_azm = std::sin(azm);

	f64 cos_alt = std::cos(alt);
	f64 cos_azm = std::cos(azm);

	f64 atan_x = std::atan2(cos_alt, sin_alt * cos_azm);
	f64 atan_y = std::atan2(cos_alt, sin_alt * sin_azm);

	i32 tx = 9000 - gsl::narrow<i32>(std::round(atan_x * 4500 / (math::num<f64>::pi / 4)));
	i32 ty = gsl::narrow<i32>(std::round(atan_y * 4500 / (math::num<f64>::pi / 4))) - 9000;

	return {tx, ty};
}

void iptsd_stylus_input(Context &ctx, const ipts::StylusData &data)
{
	StylusDevice &stylus = *ctx.devices.stylus;

	stylus.active = data.proximity;

	if (data.proximity) {
		// Convert logical to physical coordinates
		f64 x = (static_cast<f64>(data.x) / IPTS_MAX_X) * ctx.config.width;
		f64 y = (static_cast<f64>(data.y) / IPTS_MAX_Y) * ctx.config.height;

		stylus.cone->update_position(x, y);
	}

	bool btn_pen = data.proximity && !data.rubber;
	bool btn_rubber = data.proximity && data.rubber;

	const auto [tx, ty] = get_tilt(data.altitude, data.azimuth);

	stylus.emit(EV_KEY, BTN_TOUCH, data.contact);
	stylus.emit(EV_KEY, BTN_TOOL_PEN, btn_pen);
	stylus.emit(EV_KEY, BTN_TOOL_RUBBER, btn_rubber);
	stylus.emit(EV_KEY, BTN_STYLUS, data.button);

	stylus.emit(EV_ABS, ABS_X, data.x);
	stylus.emit(EV_ABS, ABS_Y, data.y);
	stylus.emit(EV_ABS, ABS_PRESSURE, data.pressure);
	stylus.emit(EV_ABS, ABS_MISC, data.timestamp);

	stylus.emit(EV_ABS, ABS_TILT_X, tx);
	stylus.emit(EV_ABS, ABS_TILT_Y, ty);

	stylus.emit(EV_SYN, SYN_REPORT, 0);
}

} // namespace iptsd::daemon
