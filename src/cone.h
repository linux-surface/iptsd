/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONE_H_
#define _IPTSD_CONE_H_

#include <stdbool.h>
#include <stdint.h>

#include "contact.h"

struct cone {
	uint64_t position_update;
	uint64_t direction_update;
	float x;
	float y;
	float dx;
	float dy;
};

void cone_set_tip(struct cone *c, float x, float y);
bool cone_is_removed(struct cone *c);
float cone_hypot(struct cone *c, float x, float y);
void cone_update_direction(struct cone *c, float x, float y);
bool cone_is_inside(struct cone *c, float x, float y);

#endif /* _IPTSD_CONE_H_ */
