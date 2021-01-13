// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdbool.h>

#include "finger.h"
#include "touch-processing.h"

static void iptsd_finger_update_from_last(struct iptsd_touch_processor *tp,
					  struct iptsd_touch_input *input,
					  struct iptsd_touch_input last)
{
	struct contact *contact = input->contact;

	float dev1 = input->ev1 - last.ev1;
	float dev2 = input->ev2 - last.ev2;

	bool is_stable =
		dev1 < tp->config.stability_threshold && dev2 < tp->config.stability_threshold;

	input->index = last.index;
	input->is_palm = contact->is_palm || last.is_palm;
	input->is_stable = is_stable;
}

static bool iptsd_finger_find_duplicates(struct iptsd_touch_processor *tp, int count, int itr)
{
	int duplicates = 0;
	int max_contacts = tp->device_info.max_contacts;

	for (int i = 0; i < count; i++) {
		bool duplicated = false;
		int base = i * max_contacts;

		if (tp->inputs[i].index == -1)
			continue;

		/*
		 * Input A is a duplicate of Input B if they have the same
		 * index, and B is closer to the last input with that index
		 * than A.
		 */
		for (int k = 0; k < count; k++) {
			int base_k = k * max_contacts;

			if (i == k)
				continue;

			if (tp->inputs[i].index != tp->inputs[k].index)
				continue;

			double dist_i = tp->distances[base + itr - 1];
			double dist_k = tp->distances[base_k + itr - 1];

			if (dist_i < dist_k)
				continue;

			duplicated = true;
			break;
		}

		if (!duplicated)
			continue;

		/*
		 * If we change the index now, the inputs that get checked
		 * after this one will think they are duplicates. Instead,
		 * we set the index to -2 and fix it up later, after all
		 * inputs have been checked.
		 */
		tp->inputs[i].index = -2;
		duplicates++;
	}

	/*
	 * If we haven't found any duplicates, we don't need to keep
	 * searching for them.
	 */
	if (duplicates == 0)
		return false;

	/*
	 * Update the index for all duplicated points.
	 *
	 * We started by using the index of the nearest input from the
	 * last cycle. Since that resulted in a duplicate we use the
	 * next-nearest point now (incremented index). We continue to do
	 * there are no duplicates anymore.
	 */
	for (int i = 0; i < max_contacts; i++) {
		if (tp->inputs[i].index != -2)
			continue;

		int index = i * max_contacts + itr;
		struct iptsd_touch_input last = tp->last[tp->indices[index]];

		iptsd_finger_update_from_last(tp, &tp->inputs[i], last);
		duplicates--;

		if (duplicates == 0)
			break;
	}

	return true;
}

void iptsd_finger_track(struct iptsd_touch_processor *tp, int count)
{
	int max_contacts = tp->device_info.max_contacts;

	/*
	 * For every current input, we calculate the distance to all
	 * previous inputs. Then we use these distances to create a sorted
	 * list of their indices, going from nearest to furthest.
	 */
	for (int i = 0; i < max_contacts; i++) {
		int base = i * max_contacts;

		for (int j = 0; j < max_contacts; j++) {
			tp->indices[base + j] = j;

			struct iptsd_touch_input current = tp->inputs[i];
			struct iptsd_touch_input last = tp->last[j];

			if (current.index == -1 || last.index == -1) {
				tp->distances[base + j] = (double)(1 << 30) + j;
				continue;
			}

			tp->distances[base + j] = iptsd_touch_processing_dist(current, last);
		}

		/* Sort the list */
		bool swapped = true;

		for (int n = max_contacts; swapped; n--) {
			swapped = false;

			for (int k = 1; k < n; k++) {
				int index_a = tp->indices[base + k];
				int index_b = tp->indices[base + k - 1];

				double dist_a = tp->distances[base + index_a];
				double dist_b = tp->distances[base + index_b];

				if (dist_a >= dist_b)
					continue;

				tp->indices[base + k] = index_b;
				tp->indices[base + k - 1] = index_a;
				swapped = true;
			}
		}
	}

	/*
	 * Choose the index of the closest previour input,
	 * and determine stability of the contact.
	 */
	for (int i = 0; i < count; i++) {
		int index = i * max_contacts;
		struct iptsd_touch_input last = tp->last[tp->indices[index]];

		iptsd_finger_update_from_last(tp, &tp->inputs[i], last);
	}

	/*
	 * The above selection will definitly lead to duplicates. For example,
	 * a new input will always get the index 0, because that is the
	 * smallest distance that gets calculated (2^30 + 0)
	 *
	 * To fix this we iterate over the inputs, search for duplicates and
	 * fix them, until every input has an unique index (or -1 which will
	 * get handled later)
	 */
	for (int j = 1; j < max_contacts; j++) {
		if (!iptsd_finger_find_duplicates(tp, count, j))
			break;
	}

	/*
	 * If by now one of the inputs still has the index -1, it is a new
	 * one, so we need to find a free index for it to use.
	 *
	 * We iterate over all inputs to find the one with index -1 first.
	 * Then we go through every possible index to check if it is already
	 * used by other inputs. If we cannot find an input using the index
	 * we assign it and continue to the next one.
	 *
	 * We also need to assign an unique slot ID to all inputs that don't
	 * touch the screen. For the remaining inputs, we simply reuse the
	 * finger index as the slot ID. The ID needs to stay the same, to
	 * prevent libinput errors.
	 */
	for (int i = 0; i < max_contacts; i++) {
		if (tp->inputs[i].index == -1)
			continue;

		tp->inputs[i].slot = tp->inputs[i].index;
		tp->free_indices[tp->inputs[i].index] = false;
	}

	for (int i = 0; i < max_contacts; i++) {
		if (tp->inputs[i].index != -1)
			continue;

		for (int k = 0; k < max_contacts; k++) {
			if (!tp->free_indices[k])
				continue;

			tp->free_indices[k] = false;
			tp->inputs[i].slot = k;
			if (i < count)
				tp->inputs[i].index = k;

			break;
		}
	}
}
