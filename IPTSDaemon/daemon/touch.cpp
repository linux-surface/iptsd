// SPDX-License-Identifier: GPL-2.0-or-later

#include "touch.hpp"

#include "context.hpp"
#include "devices.hpp"

#include <contacts/finder.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.hpp>

#include <vector>

namespace iptsd::daemon {

static bool check_cone(const Context &ctx, const contacts::Contact &contact)
{
	const TouchDevice &touch = *ctx.devices.touch;

	// Convert relative to physical coordinates
	f64 x = contact.x * ctx.config.width;
	f64 y = contact.y * ctx.config.height;

	return touch.cone->check(x, y);
}

static void update_cone(Context &ctx, const contacts::Contact &contact)
{
	TouchDevice &touch = *ctx.devices.touch;

	if (contact.valid)
		return;

	// Convert relative to physical coordinates
	f64 x = contact.x * ctx.config.width;
	f64 y = contact.y * ctx.config.height;

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

bool iptsd_touch_input(Context &ctx, const ipts::Heatmap &data, IPTSHIDReport &report)
{
	TouchDevice &touch = *ctx.devices.touch;

	// Make sure that all buffers have the correct size
	touch.finder.resize(index2_t {data.dim.width, data.dim.height});

	// Normalize and invert the heatmap data.
	std::transform(data.data.begin(), data.data.end(), touch.finder.data().begin(),
		       [&](auto v) {
			       f32 val = static_cast<f32>(v - data.dim.z_min) /
					 static_cast<f32>(data.dim.z_max - data.dim.z_min);

			       return 1.0f - val;
		       });

	// Search for contacts
	const std::vector<contacts::Contact> &contacts = touch.finder.search();

	// Update stylus rejection cones
	for (const auto &contact : contacts)
		update_cone(ctx, contact);

    if (check_blocked(ctx, contacts))
        return false;
    
    int contact_cnt = 0;
    for (const auto &contact : contacts) {
        if (contact.active && !contact.stable && ctx.config.touch_check_stability)
            continue;
        if (check_lift(ctx, contact))
            continue;
        IPTSFingerReport &finger = report.report.touch.fingers[contact_cnt];
        finger.touch = true;
        finger.contact_id = contact.index;
        finger.x = gsl::narrow_cast<u16>(contact.x * IPTS_TOUCH_MAX_VALUE);
        finger.y = gsl::narrow_cast<u16>(contact.y * IPTS_TOUCH_MAX_VALUE);
        
        contact_cnt++;
    }
    report.report_id = IPTS_TOUCH_REPORT_ID;
    report.report.touch.contact_num = contact_cnt;
    
    return true;
}

} // namespace iptsd::daemon
