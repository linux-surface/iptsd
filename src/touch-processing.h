/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_TOUCH_PROCESSING_H_
#define _IPTSD_TOUCH_PROCESSING_H_

#include <stdbool.h>

#include "cone.h"
#include "config.h"
#include "constants.h"
#include "contact.h"
#include "heatmap.h"
#include "protocol.h"

struct iptsd_touch_input {
	int x;
	int y;
	int major;
	int minor;
	int orientation;
	float ev1;
	float ev2;
	int index;
	int slot;
	bool is_stable;
	bool is_palm;

	struct contact *contact;
};

struct iptsd_touch_processor {
	struct heatmap hm;
	struct contact *contacts;
	struct iptsd_touch_input *inputs;
	struct iptsd_touch_input *last;
	struct cone rejection_cones[IPTSD_MAX_STYLI];

	bool *free_indices;
	double *distances;
	int *indices;

	struct iptsd_config config;
	struct ipts_device_info device_info;
};

double iptsd_touch_processing_dist(struct iptsd_touch_input input, struct iptsd_touch_input other);
void iptsd_touch_processing_inputs(struct iptsd_touch_processor *tp, struct heatmap *hm);
struct heatmap *iptsd_touch_processing_get_heatmap(struct iptsd_touch_processor *tp, int w, int h);
int iptsd_touch_processing_init(struct iptsd_touch_processor *tp);
void iptsd_touch_processing_free(struct iptsd_touch_processor *tp);

#endif /* _IPTSD_TOUCH_PROCESSING_H_ */
