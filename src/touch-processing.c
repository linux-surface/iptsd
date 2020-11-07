// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include "contact.h"
#include "finger.h"
#include "heatmap.h"
#include "touch-processing.h"
#include "utils.h"

double iptsd_touch_processing_dist(struct iptsd_touch_input input,
		struct iptsd_touch_input other)
{
	double dx = (double)input.x - (double)other.x;
	double dy = (double)input.y - (double)other.y;

	return sqrt(dx * dx + dy * dy);
}

static void iptsd_touch_processing_reset(struct iptsd_touch_input *input)
{
	input->x = 0;
	input->y = 0;
	input->ev1 = 0;
	input->ev2 = 0;
	input->index = -1;
	input->is_stable = false;
	input->is_palm = false;
}

static void iptsd_touch_processing_save(struct iptsd_touch_processor *tp)
{
	for (int i = 0; i < tp->max_contacts; i++)
		tp->free_indices[i] = true;

	for (int i = 0; i < tp->max_contacts; i++) {
		tp->last[i] = tp->inputs[i];

		if (tp->inputs[i].index == -1)
			continue;

		tp->free_indices[tp->inputs[i].index] = false;
	}
}

void iptsd_touch_processing_inputs(struct iptsd_touch_processor *tp,
		struct heatmap *hm)
{
	float average = heatmap_average(hm);

	for (int i = 0; i < hm->size; i++) {
		if (hm->data[i] < average)
			hm->data[i] = average - hm->data[i];
		else
			hm->data[i] = 0;
	}

	int count = contacts_get(hm, tp->contacts, tp->max_contacts);
	iptsd_touch_processing_get_palms(tp, count);

	for (int i = 0; i < count; i++) {
		float x = tp->contacts[i].x / (float)(hm->width - 1);
		float y = tp->contacts[i].y / (float)(hm->height - 1);

		if (tp->invert_x)
			x = 1 - x;

		if (tp->invert_y)
			y = 1 - y;

		tp->inputs[i].x = (int)(x * 9600);
		tp->inputs[i].y = (int)(y * 7200);
		tp->inputs[i].ev1 = tp->contacts[i].ev1;
		tp->inputs[i].ev2 = tp->contacts[i].ev2;
		tp->inputs[i].index = i;
		tp->inputs[i].slot = i;
		tp->inputs[i].is_stable = false;
		tp->inputs[i].is_palm = tp->contacts[i].is_palm;
		tp->inputs[i].contact = &tp->contacts[i];
	}

	for (int i = count; i < tp->max_contacts; i++) {
		iptsd_touch_processing_reset(&tp->inputs[i]);
		tp->inputs[i].slot = i;
		tp->inputs[i].contact = &tp->contacts[i];
	}

	iptsd_finger_track(tp, count);
	iptsd_touch_processing_save(tp);
}

struct heatmap *iptsd_touch_processing_get_heatmap(
		struct iptsd_touch_processor *tp, int width, int height)
{
	if (tp->hm.width == width && tp->hm.height == height)
		return &tp->hm;

	heatmap_free(&tp->hm);
	tp->hm.width = width;
	tp->hm.height = height;
	heatmap_init(&tp->hm);

	return &tp->hm;
}

int iptsd_touch_processing_init(struct iptsd_touch_processor *tp)
{
	tp->contacts = calloc(tp->max_contacts, sizeof(struct contact));
	if (!tp->contacts)
		return -ENOMEM;

	tp->inputs = calloc(tp->max_contacts, sizeof(struct iptsd_touch_input));
	if (!tp->inputs)
		return -ENOMEM;

	tp->last = calloc(tp->max_contacts, sizeof(struct iptsd_touch_input));
	if (!tp->last)
		return -ENOMEM;

	tp->free_indices = calloc(tp->max_contacts, sizeof(bool));
	if (!tp->free_indices)
		return -ENOMEM;

	tp->distances = calloc(tp->max_contacts * tp->max_contacts,
			sizeof(double));
	if (!tp->distances)
		return -ENOMEM;

	tp->indices = calloc(tp->max_contacts * tp->max_contacts, sizeof(int));
	if (!tp->indices)
		return -ENOMEM;

	for (int i = 0; i < tp->max_contacts; i++) {
		iptsd_touch_processing_reset(&tp->last[i]);
		tp->last[i].slot = i;
		tp->last[i].contact = &tp->contacts[i];

		tp->free_indices[i] = true;
	}

	return 0;
}

void iptsd_touch_processing_free(struct iptsd_touch_processor *tp)
{
	if (tp->contacts)
		free(tp->contacts);

	if (tp->inputs)
		free(tp->inputs);

	if (tp->last)
		free(tp->last);
	
	if (tp->free_indices)
		free(tp->free_indices);

	if (tp->distances)
		free(tp->distances);

	if (tp->indices)
		free(tp->indices);

}

void iptsd_touch_rejection_cone_set_tip(struct iptsd_touch_processor *tp,
		uint32_t serial, int x, int y)
{
	struct iptsd_touch_rejection_cone *cone = tp->rejection_cones;
	while (cone < tp->rejection_cones + IPTSD_MAX_STYLI &&
			cone->pen_serial != serial)
		cone++;
	
	if (cone == tp->rejection_cones + IPTSD_MAX_STYLI) {
		iptsd_err(-1, "No cone rejection found for stylus.");
		return;
	}

	float fx = x / 9600.0;
	float fy = y / 7200.0;

	if (tp->invert_x)
		fx = 1 - fx;

	if (tp->invert_y)
		fy = 1 - fy;

	cone->x = fx * (tp->hm.width - 1);
	cone->y = fy * (tp->hm.height - 1);
	cone->position_update_timestamp = iptsd_utils_msec_timestamp();
}

void iptsd_touch_rejection_cone_update_direction(
		struct iptsd_touch_processor *tp, struct contact *palm,
		uint64_t timestamp)
{
	struct iptsd_touch_rejection_cone *cone = NULL;
	float d = INFINITY;

	// find closest cone (by center)
	for (int i = 0; i < tp->n_cones; i++) {
		struct iptsd_touch_rejection_cone *curr_cone = &tp->rejection_cones[i];
		if (curr_cone->position_update_timestamp + 300 < timestamp)
			continue; // pon lifted
		float curr_d = hypotf(curr_cone->x - palm->x, curr_cone->y - palm->y);
		if (curr_d < d) {
			d = curr_d;
			cone = curr_cone;
		}
	}

	if (cone == NULL)
		return;

	// update cone direction vector
	float time_diff = (timestamp - cone->direction_update_timestamp) / 1000.0;
	float weight = exp2(-time_diff);
	
	float dx = (palm->x - cone->x) / (d + 1e-6); // prevent division by 0
	float dy = (palm->y - cone->y) / (d + 1e-6);

	cone->dx = weight * cone->dx + dx;
	cone->dy = weight * cone->dy + dy;

	// normalize cone direction vector
	d = (hypotf(cone->dx, cone->dy) + 1e-6);
	cone->dx /= d;
	cone->dy /= d;

	cone->direction_update_timestamp = timestamp;
}

bool iptsd_touch_rejection_cone_is_inside(
		struct iptsd_touch_processor *tp, struct contact *input,
		uint64_t timestamp)
{
	for (int i = 0; i < tp->n_cones; i++) {
		struct iptsd_touch_rejection_cone *cone = tp->rejection_cones + i;
		if (cone->position_update_timestamp + 300 < timestamp)
			continue; // pen lifted
		
		float dx = input->x - cone->x;
		float dy = input->y - cone->y;
		float d = hypotf(dx, dy);

		if (d > CONE_DISTANCE_THRESHOLD)
			continue; // too far from pen tip

		if (dx * cone->dx + dy * cone->dy > CONE_COS_THRESHOLD * d)
			return true;
	}

	return false;
}


void iptsd_touch_processing_get_palms(struct iptsd_touch_processor *tp,
		int contacts_count)
{
	uint64_t timestamp = iptsd_utils_msec_timestamp();
	for (int i = 0; i < contacts_count; i++) {
		float vx = tp->contacts[i].ev1;
		float vy = tp->contacts[i].ev2;
		float max_v = tp->contacts[i].max_v;

		// Regular touch
		if (vx < 0.6 || (vx < 1.0 && max_v > 80))
			continue;

		// Thumb
		if ((vx < 1.25 || (vx < 3.5 && max_v > 90)) && vx/vy > 1.8)
			continue;

		tp->contacts[i].is_palm = true;
		iptsd_touch_rejection_cone_update_direction(tp, &tp->contacts[i],
				timestamp);

		for (int j = 0; j < contacts_count; j++) {
			if (tp->contacts[j].is_palm)
				continue;

			if (contact_near(tp->contacts[j], tp->contacts[i]))
				tp->contacts[j].is_palm = true;
		}
	}
	
	for (int i = 0; i < contacts_count; i++) {
		if (!tp->contacts[i].is_palm) {
			if (iptsd_touch_rejection_cone_is_inside(tp, &tp->contacts[i],
					timestamp))
				tp->contacts[i].is_palm = true;
		}
	}
}
