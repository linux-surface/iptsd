// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include "contact.h"
#include "finger.h"
#include "heatmap.h"
#include "protocol.h"
#include "touch-processing.h"
#include "utils.h"

double iptsd_touch_processing_dist(struct iptsd_touch_input input, struct iptsd_touch_input other)
{
	double dx = (double)input.x - (double)other.x;
	double dy = (double)input.y - (double)other.y;

	return sqrt(dx * dx + dy * dy);
}

static void iptsd_touch_processing_update_cone(struct iptsd_touch_processor *tp,
					       struct contact *palm)
{
	struct cone *cone = NULL;
	float d = INFINITY;

	// find closest cone (by center)
	for (int i = 0; i < IPTSD_MAX_STYLI; i++) {
		struct cone *current = &tp->rejection_cones[i];

		// This cone has never seen a position update, so its inactive
		if (current->position_update == 0)
			continue;

		if (cone_is_removed(current))
			continue;

		float current_d = cone_hypot(current, palm->x, palm->y);

		if (current_d < d) {
			d = current_d;
			cone = current;
		}
	}

	if (!cone)
		return;

	cone_update_direction(cone, palm->x, palm->y);
}

static bool iptsd_touch_processing_check_cone(struct iptsd_touch_processor *tp,
					      struct contact *input)
{
	for (int i = 0; i < IPTSD_MAX_STYLI; i++) {
		struct cone *cone = &tp->rejection_cones[i];

		if (cone_is_inside(cone, input->x, input->y))
			return true;
	}

	return false;
}

static void iptsd_touch_processing_get_palms(struct iptsd_touch_processor *tp, int count)
{
	for (int i = 0; i < count; i++) {
		float vx = tp->contacts[i].ev1;
		float vy = tp->contacts[i].ev2;
		float max_v = tp->contacts[i].max_v;

		// Regular touch
		if (vx < 0.6 || (vx < 1.0 && max_v > 80))
			continue;

		// Thumb
		if ((vx < 1.25 || (vx < 3.5 && max_v > 90)) && vx / vy > 1.8)
			continue;

		tp->contacts[i].is_palm = true;
		iptsd_touch_processing_update_cone(tp, &tp->contacts[i]);

		for (int j = 0; j < count; j++) {
			if (tp->contacts[j].is_palm)
				continue;

			if (contact_near(tp->contacts[j], tp->contacts[i]))
				tp->contacts[j].is_palm = true;
		}
	}

	for (int i = 0; i < count; i++) {
		if (tp->contacts[i].is_palm)
			continue;

		if (iptsd_touch_processing_check_cone(tp, &tp->contacts[i]))
			tp->contacts[i].is_palm = true;
	}
}

static void iptsd_touch_processing_reset(struct iptsd_touch_input *input)
{
	input->x = 0;
	input->y = 0;
	input->major = 0;
	input->minor = 0;
	input->orientation = 0;
	input->ev1 = 0;
	input->ev2 = 0;
	input->index = -1;
	input->is_stable = false;
	input->is_palm = false;
}

static void iptsd_touch_processing_save(struct iptsd_touch_processor *tp)
{
	for (int i = 0; i < tp->device_info.max_contacts; i++)
		tp->free_indices[i] = true;

	for (int i = 0; i < tp->device_info.max_contacts; i++)
		tp->last[i] = tp->inputs[i];
}

void iptsd_touch_processing_inputs(struct iptsd_touch_processor *tp, struct heatmap *hm)
{
	float average = heatmap_average(hm);

	for (int i = 0; i < hm->size; i++) {
		if (hm->data[i] < average)
			hm->data[i] = average - hm->data[i];
		else
			hm->data[i] = 0;
	}

	int count = contacts_get(hm, tp->contacts, tp->device_info.max_contacts);

	for (int i = 0; i < count; i++) {
		float x = tp->contacts[i].x / (hm->width - 1);
		float y = tp->contacts[i].y / (hm->height - 1);

		if (tp->config.invert_x)
			x = 1 - x;

		if (tp->config.invert_y)
			y = 1 - y;

		tp->contacts[i].x = x * tp->config.width;
		tp->contacts[i].y = y * tp->config.height;
	}

	iptsd_touch_processing_get_palms(tp, count);

	for (int i = 0; i < count; i++) {
		float x = tp->contacts[i].x / tp->config.width;
		float y = tp->contacts[i].y / tp->config.height;

		// ev1 is always the larger eigenvalue.
		float orientation = tp->contacts[i].angle / M_PI * 180;
		float maj = 4 * sqrtf(tp->contacts[i].ev1) / hm->diagonal;
		float min = 4 * sqrtf(tp->contacts[i].ev2) / hm->diagonal;

		tp->inputs[i].x = (int)(x * IPTS_MAX_X);
		tp->inputs[i].y = (int)(y * IPTS_MAX_Y);
		tp->inputs[i].major = (int)(maj * IPTS_DIAGONAL);
		tp->inputs[i].minor = (int)(min * IPTS_DIAGONAL);
		tp->inputs[i].orientation = (int)orientation;
		tp->inputs[i].ev1 = tp->contacts[i].ev1;
		tp->inputs[i].ev2 = tp->contacts[i].ev2;
		tp->inputs[i].index = i;
		tp->inputs[i].slot = i;
		tp->inputs[i].is_stable = false;
		tp->inputs[i].is_palm = tp->contacts[i].is_palm;
		tp->inputs[i].contact = &tp->contacts[i];
	}

	for (int i = count; i < tp->device_info.max_contacts; i++) {
		iptsd_touch_processing_reset(&tp->inputs[i]);
		tp->inputs[i].slot = i;
		tp->inputs[i].contact = &tp->contacts[i];
	}

	iptsd_finger_track(tp, count);
	iptsd_touch_processing_save(tp);
}

struct heatmap *iptsd_touch_processing_get_heatmap(struct iptsd_touch_processor *tp, int w, int h)
{
	if (tp->hm.width == w && tp->hm.height == h)
		return &tp->hm;

	heatmap_free(&tp->hm);
	tp->hm.width = w;
	tp->hm.height = h;
	tp->hm.touch_threshold = tp->config.touch_threshold;
	heatmap_init(&tp->hm);

	return &tp->hm;
}

int iptsd_touch_processing_init(struct iptsd_touch_processor *tp)
{
	int max_contacts = tp->device_info.max_contacts;

	tp->contacts = calloc(max_contacts, sizeof(struct contact));
	if (!tp->contacts)
		return -ENOMEM;

	tp->inputs = calloc(max_contacts, sizeof(struct iptsd_touch_input));
	if (!tp->inputs)
		return -ENOMEM;

	tp->last = calloc(max_contacts, sizeof(struct iptsd_touch_input));
	if (!tp->last)
		return -ENOMEM;

	tp->free_indices = calloc(max_contacts, sizeof(bool));
	if (!tp->free_indices)
		return -ENOMEM;

	tp->distances = calloc(max_contacts * max_contacts, sizeof(double));
	if (!tp->distances)
		return -ENOMEM;

	tp->indices = calloc(max_contacts * max_contacts, sizeof(int));
	if (!tp->indices)
		return -ENOMEM;

	for (int i = 0; i < max_contacts; i++) {
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

	heatmap_free(&tp->hm);
}
