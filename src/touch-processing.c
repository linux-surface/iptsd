// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <math.h>
#include <stdlib.h>

#include "contact.h"
#include "finger.h"
#include "heatmap.h"
#include "touch-processing.h"

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
	contacts_get_palms(tp->contacts, tp->max_contacts);

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

