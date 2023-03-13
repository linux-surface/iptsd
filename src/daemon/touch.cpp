// SPDX-License-Identifier: GPL-2.0-or-later

#include "touch.hpp"

#include "contacts/contact.hpp"
#include "context.hpp"
#include "devices.hpp"

#include <contacts/finder.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.hpp>

#include <algorithm>
#include <iterator>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <pstl/glue_algorithm_defs.h>
#include <vector>

namespace iptsd::daemon {

static bool check_cone(const Context &ctx, const contacts::Contact<f32> &contact)
{
	const TouchDevice &touch = *ctx.devices.touch;

	// Convert relative to physical coordinates
	const f32 x = contact.mean.x() * ctx.config.width;
	const f32 y = contact.mean.y() * ctx.config.height;

	return touch.cone->check(x, y);
}

static void update_cone(Context &ctx, const contacts::Contact<f32> &contact)
{
	const TouchDevice &touch = *ctx.devices.touch;

	if (contact.valid)
		return;

	// Convert relative to physical coordinates
	const f32 x = contact.mean.x() * ctx.config.width;
	const f32 y = contact.mean.y() * ctx.config.height;

	// The cone has never seen a position update, so its inactive
	if (!touch.cone->alive())
		return;

	if (!touch.cone->active())
		return;

	touch.cone->update_direction(x, y);
}

static bool check_blocked(const Context &ctx, const std::vector<contacts::Contact<f32>> &contacts)
{
	bool blocked = false;

	for (const auto &p : contacts)
		blocked |= !p.valid && ctx.config.touch_disable_on_palm;

	if (ctx.devices.stylus->active && ctx.config.touch_disable_on_stylus)
		blocked = true;

	return blocked;
}

static bool check_lift(const Context &ctx, const contacts::Contact<f32> &contact)
{
	// Lift inactive contacts
	// if (!contact.active)
	//	return true;

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

static void emit_multi(const TouchDevice &dev, const contacts::Contact<f32> &contact)
{
	const i32 index = gsl::narrow<i32>(contact.index.value());

	const i32 x = gsl::narrow<i32>(std::round(contact.mean.x() * IPTS_MAX_X));
	const i32 y = gsl::narrow<i32>(std::round(contact.mean.y() * IPTS_MAX_Y));

	const i32 angle = gsl::narrow<i32>(std::round(contact.orientation * 180));
	const i32 major = gsl::narrow<i32>(std::round(contact.size.maxCoeff() * IPTS_DIAGONAL));
	const i32 minor = gsl::narrow<i32>(std::round(contact.size.minCoeff() * IPTS_DIAGONAL));

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

static void emit_single(const TouchDevice &dev, const contacts::Contact<f32> &contact)
{
	const i32 x = gsl::narrow<i32>(std::round(contact.mean.x() * IPTS_MAX_X));
	const i32 y = gsl::narrow<i32>(std::round(contact.mean.y() * IPTS_MAX_Y));

	dev.emit(EV_KEY, BTN_TOUCH, 1);
	dev.emit(EV_ABS, ABS_X, x);
	dev.emit(EV_ABS, ABS_Y, y);
}

static void handle_single(const Context &ctx, const std::vector<contacts::Contact<f32>> &contacts)
{
	const TouchDevice &touch = *ctx.devices.touch;
	const bool blocked = check_blocked(ctx, contacts);

	if (blocked || contacts.empty()) {
		lift_single(touch);
		return;
	}

	for (const contacts::Contact<f32> &contact : contacts) {
		if (!contact.stable && ctx.config.touch_check_stability)
			return;

		if (check_lift(ctx, contact))
			continue;

		emit_single(touch, contact);
		return;
	}

	lift_single(touch);
}

static void handle_multi(const Context &ctx, const std::vector<contacts::Contact<f32>> &contacts)
{
	TouchDevice &touch = *ctx.devices.touch;
	const bool blocked = check_blocked(ctx, contacts);

	for (const auto &contact : contacts) {
		if (!contact.stable && ctx.config.touch_check_stability)
			continue;

		if (!contact.index.has_value())
			continue;

		const usize index = contact.index.value();
		touch.emit(EV_ABS, ABS_MT_SLOT, gsl::narrow<i32>(index));

		if (check_lift(ctx, contact) || blocked) {
			lift_multi(touch);
			touch.current.erase(index);
		} else {
			emit_multi(touch, contact);
		}
	}

	for (const usize &index : touch.lift) {
		touch.emit(EV_ABS, ABS_MT_SLOT, gsl::narrow<i32>(index));
		lift_multi(touch);
	}
}

void iptsd_touch_input(Context &ctx, const ipts::Heatmap &data)
{
	TouchDevice &touch = *ctx.devices.touch;
	std::vector<contacts::Contact<f32>> &contacts = touch.contacts;

	const Eigen::Index rows = index_cast(data.dim.height);
	const Eigen::Index cols = index_cast(data.dim.width);

	// Make sure the heatmap buffer has the right size
	if (touch.heatmap.rows() != rows || touch.heatmap.cols() != cols)
		touch.heatmap.conservativeResize(data.dim.height, data.dim.width);

	// Map the buffer to an Eigen container
	Eigen::Map<const Image<u8>> mapped {data.data.data(), rows, cols};

	const f32 range = static_cast<f32>(data.dim.z_max - data.dim.z_min);

	// Normalize and invert the heatmap.
	touch.heatmap = 1.0f - (mapped.cast<f32>() - static_cast<f32>(data.dim.z_min)) / range;

	// Search for contacts
	touch.finder.find(touch.heatmap, contacts);

	touch.current.clear();

	for (const auto &contact : contacts) {
		// Update stylus rejection cones
		update_cone(ctx, contact);

		if (!contact.index.has_value())
			continue;

		touch.current.insert(contact.index.value());
	}

	touch.lift.clear();

	std::set_difference(touch.last.cbegin(), touch.last.cend(), touch.current.cbegin(),
			    touch.current.cend(), std::inserter(touch.lift, touch.lift.begin()));

	handle_multi(ctx, contacts);
	handle_single(ctx, contacts);

	touch.emit(EV_SYN, SYN_REPORT, 0);

	std::swap(touch.current, touch.last);
}

} // namespace iptsd::daemon
