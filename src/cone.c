// SPDX-License-Identifier: GPL-2.0-or-later

#include <math.h>

#include "cone.h"
#include "constants.h"
#include "contact.h"
#include "utils.h"

void cone_set_tip(struct cone *c, float x, float y)
{
	c->x = x;
	c->y = y;

	int ret = iptsd_utils_msec_timestamp(&c->position_update);
	if (ret < 0)
		iptsd_err(ret, "Failed to get timestamp");
}

bool cone_is_removed(struct cone *c)
{
	uint64_t timestamp;

	int ret = iptsd_utils_msec_timestamp(&timestamp);
	if (ret < 0) {
		iptsd_err(ret, "Failed to get timestamp");
		return false;
	}

	if (c->position_update + 300 > timestamp)
		return false;

	return true;
}

float cone_hypot(struct cone *c, float x, float y)
{
	return hypotf(c->x - x, c->y - y);
}

void cone_update_direction(struct cone *c, float x, float y)
{
	uint64_t timestamp;

	int ret = iptsd_utils_msec_timestamp(&timestamp);
	if (ret < 0) {
		iptsd_err(ret, "Failed to get timestamp");
		return;
	}

	float time_diff = (timestamp - c->direction_update) / 1000.0;
	float weight = exp2f(-time_diff);

	float d = hypotf(c->x - x, c->y - y);

	float dx = (x - c->x) / (d + 1E-6);
	float dy = (y - c->y) / (d + 1E-6);

	c->dx = weight * c->dx + dx;
	c->dy = weight * c->dy + dy;

	// Normalize cone direction vector
	d = hypotf(c->dx, c->dy) + 1E-6;
	c->dx /= d;
	c->dy /= d;

	c->direction_update = timestamp;
}

bool cone_is_inside(struct cone *c, float x, float y)
{
	if (cone_is_removed(c))
		return false;

	float dx = x - c->x;
	float dy = y - c->y;
	float d = hypotf(dx, dy);

	if (d > CONE_DISTANCE_THRESHOLD)
		return false;

	if (dx * c->dx + dy * c->dy > CONE_COS_THRESHOLD * d)
		return true;

	return false;
}
