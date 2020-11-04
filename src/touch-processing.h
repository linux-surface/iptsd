/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_TOUCH_PROCESSING_H_
#define _IPTSD_TOUCH_PROCESSING_H_

#include <stdbool.h>

#include "contact.h"
#include "heatmap.h"

#define CONE_COS_THRESHOLD 0.8660 // cos(30 degrees)
#define CONE_DISTANCE_THRESHOLD 20

struct iptsd_touch_input {
	int x;
	int y;
	float ev1;
	float ev2;
	int index;
	int slot;
	bool is_stable;
	bool is_palm;

	struct contact *contact;
};

struct iptsd_touch_rejection_cone {
	unsigned position_update_timestamp; // should be in msec
	unsigned direction_update_timestamp; // should be in msec
	float x;
	float y;
	float dx;
	float dy;
};

struct iptsd_touch_processor {
	struct heatmap hm;
	struct contact *contacts;
	struct iptsd_touch_input *inputs;
	struct iptsd_touch_input *last;
	struct iptsd_touch_rejection_cone *rejection_cone;
	bool *free_indices;
	double *distances;
	int *indices;

	int max_contacts;
	bool invert_x;
	bool invert_y;
};

double iptsd_touch_processing_dist(struct iptsd_touch_input input,
		struct iptsd_touch_input other);
void iptsd_touch_processing_inputs(struct iptsd_touch_processor *tp,
		struct heatmap *hm);
struct heatmap *iptsd_touch_processing_get_heatmap(
		struct iptsd_touch_processor *tp, int width, int height);
int iptsd_touch_processing_init(struct iptsd_touch_processor *tp);
void iptsd_touch_processing_free(struct iptsd_touch_processor *tp);

void iptsd_touch_rejection_cone_set_tip(
	struct iptsd_touch_processor *cone, int x, int y, unsigned timestamp);
void iptsd_touch_rejection_cone_update_direction(
	struct iptsd_touch_rejection_cone *cone, struct contact *palm,
	unsigned timestamp);
int iptsd_touch_rejection_cone_is_inside(
	struct iptsd_touch_rejection_cone *cone, struct contact *input,
	unsigned timestamp);

#endif /* _IPTSD_TOUCH_PROCESSING_H_ */

