/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_TOUCH_PROCESSING_H_
#define _IPTSD_TOUCH_PROCESSING_H_

#include <stdbool.h>

#include "constants.h"
#include "contact.h"
#include "heatmap.h"

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
	uint32_t pen_serial;
	uint64_t position_update_timestamp; // should be in msec
	uint64_t direction_update_timestamp; // should be in msec
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
	struct iptsd_touch_rejection_cone rejection_cones[IPTSD_MAX_STYLI];
	int n_cones;
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

void iptsd_touch_rejection_cone_set_tip(struct iptsd_touch_processor *tp,
		uint32_t serial, int x, int y);
void iptsd_touch_rejection_cone_update_direction(
		struct iptsd_touch_processor *tp, struct contact *palm,
		uint64_t timestamp);
bool iptsd_touch_rejection_cone_is_inside(
		struct iptsd_touch_processor *tp, struct contact *input,
		uint64_t timestamp);

void iptsd_touch_processing_get_palms(struct iptsd_touch_processor *tp,
		int contacts_count);

#endif /* _IPTSD_TOUCH_PROCESSING_H_ */

