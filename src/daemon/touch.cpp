// SPDX-License-Identifier: GPL-2.0-or-later

#include "touch.hpp"

#include "context.hpp"
#include "devices.hpp"

#include <contacts/finder.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.hpp>

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <vector>

namespace iptsd::daemon {

static bool check_cone(const Context &ctx, const contacts::Contact &contact)
{
	const TouchDevice &touch = *ctx.devices.touch;

	// Convert relative to physical coordinates
	const f64 x = contact.x * ctx.config.width;
	const f64 y = contact.y * ctx.config.height;

	return touch.cone->check(x, y);
}

static void update_cone(Context &ctx, const contacts::Contact &contact)
{
	const TouchDevice &touch = *ctx.devices.touch;

	if (contact.valid)
		return;

	// Convert relative to physical coordinates
	const f64 x = contact.x * ctx.config.width;
	const f64 y = contact.y * ctx.config.height;

	// The cone has never seen a position update, so its inactive
	if (!touch.cone->alive())
		return;

	if (!touch.cone->active())
		return;

	touch.cone->update_direction(x, y);
}

static bool check_blocked(const Context &ctx, const std::vector<contacts::Contact> &contacts)
{
	bool blocked = false;

	for (const auto &p : contacts)
		blocked |= !p.valid && ctx.config.touch_disable_on_palm;

	if (ctx.devices.stylus->active && ctx.config.touch_disable_on_stylus)
		blocked = true;

	return blocked;
}

static bool check_lift(const Context &ctx, const contacts::Contact &contact)
{
	// Lift inactive contacts
	if (!contact.active)
		return true;

	// Lift invalid contacts
	if (!contact.valid)
		return true;

	// Lift contacts that are blocked by a rejection cone
	if (ctx.config.touch_check_cone && check_cone(ctx, contact))
		return true;

	return false;
}

static void lift_multi(const TouchDevice &dev)
{
	dev.emit(EV_ABS, ABS_MT_TRACKING_ID, -1);
}

static void emit_multi(const TouchDevice &dev, const contacts::Contact &contact)
{
	const i32 index = gsl::narrow<i32>(contact.index);
	const i32 x = gsl::narrow<i32>(std::round(contact.x * IPTS_MAX_X));
	const i32 y = gsl::narrow<i32>(std::round(contact.y * IPTS_MAX_Y));

	const i32 angle = gsl::narrow<i32>(std::round(contact.angle * (180 / math::num<f64>::pi)));
	const i32 major = gsl::narrow<i32>(std::round(contact.major * IPTS_DIAGONAL));
	const i32 minor = gsl::narrow<i32>(std::round(contact.minor * IPTS_DIAGONAL));

	dev.emit(EV_ABS, ABS_MT_TRACKING_ID, index);
	dev.emit(EV_ABS, ABS_MT_POSITION_X, x);
	dev.emit(EV_ABS, ABS_MT_POSITION_Y, y);

	dev.emit(EV_ABS, ABS_MT_ORIENTATION, angle);
	dev.emit(EV_ABS, ABS_MT_TOUCH_MAJOR, major);
	dev.emit(EV_ABS, ABS_MT_TOUCH_MINOR, minor);
}

static void lift_single(const TouchDevice &dev)
{
	dev.emit(EV_KEY, BTN_TOUCH, 0);
}

static void emit_single(const TouchDevice &dev, const contacts::Contact &contact)
{
	const i32 x = gsl::narrow<i32>(std::round(contact.x * IPTS_MAX_X));
	const i32 y = gsl::narrow<i32>(std::round(contact.y * IPTS_MAX_Y));

	dev.emit(EV_KEY, BTN_TOUCH, 1);
	dev.emit(EV_ABS, ABS_X, x);
	dev.emit(EV_ABS, ABS_Y, y);
}

static void handle_single(const Context &ctx, const std::vector<contacts::Contact> &contacts)
{
	const TouchDevice &touch = *ctx.devices.touch;
	const bool blocked = check_blocked(ctx, contacts);

	for (const auto &contact : contacts) {
		if (contact.active && !contact.stable && ctx.config.touch_check_stability)
			return;

		if (check_lift(ctx, contact) || blocked)
			continue;

		emit_single(touch, contact);
		return;
	}

	lift_single(touch);
}

static void handle_multi(const Context &ctx, const std::vector<contacts::Contact> &contacts)
{
	const TouchDevice &touch = *ctx.devices.touch;
	const bool blocked = check_blocked(ctx, contacts);

	for (const auto &contact : contacts) {
		touch.emit(EV_ABS, ABS_MT_SLOT, gsl::narrow<i32>(contact.index));

		if (contact.active && !contact.stable && ctx.config.touch_check_stability)
			continue;

		if (check_lift(ctx, contact) || blocked) {
			lift_multi(touch);
		} else {
			emit_multi(touch, contact);
		}
	}
}

void iptsd_touch_input(Context &ctx, const ipts::Heatmap &data)
{
	TouchDevice &touch = *ctx.devices.touch;

	// Make sure that all buffers have the correct size
	touch.finder.resize(index2_t {data.dim.width, data.dim.height});

	// Normalize and invert the heatmap data.
	std::transform(data.data.begin(), data.data.end(), touch.finder.data().begin(), [&](f32 v) {
		const f32 val = (v - static_cast<f32>(data.dim.z_min)) /
				static_cast<f32>(data.dim.z_max - data.dim.z_min);

		return 1.0f - val;
	});

	// Search for contacts
	const std::vector<contacts::Contact> &contacts = touch.finder.search();

	// Update stylus rejection cones
	for (const auto &contact : contacts)
		update_cone(ctx, contact);

	handle_multi(ctx, contacts);
	handle_single(ctx, contacts);

	touch.emit(EV_SYN, SYN_REPORT, 0);
}

} // namespace iptsd::daemon
