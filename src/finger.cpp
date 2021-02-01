// SPDX-License-Identifier: GPL-2.0-or-later

#include "finger.hpp"

#include "config.hpp"
#include "contact.hpp"
#include "touch-processing.hpp"
#include "types.hpp"

#include <cstddef>
#include <iterator>
#include <memory>
#include <vector>

static void update_from_last(TouchProcessor *tp, TouchInput *input, TouchInput last)
{
	Contact *contact = input->contact;

	f32 dev1 = input->ev1 - last.ev1;
	f32 dev2 = input->ev2 - last.ev2;

	bool is_stable =
		dev1 < tp->config->stability_threshold && dev2 < tp->config->stability_threshold;

	input->index = last.index;
	input->is_palm = contact->is_palm || last.is_palm;
	input->is_stable = is_stable;
}

static bool find_duplicates(TouchProcessor *tp, size_t count, size_t itr)
{
	u8 duplicates = 0;
	size_t size = std::size(tp->inputs);

	for (size_t i = 0; i < count; i++) {
		bool duplicated = false;
		size_t base = i * size;

		if (tp->inputs[i].index == -1)
			continue;

		/*
		 * Input A is a duplicate of Input B if they have the same
		 * index, and B is closer to the last input with that index
		 * than A.
		 */
		for (size_t k = 0; k < count; k++) {
			size_t base_k = k * size;

			if (i == k)
				continue;

			if (tp->inputs[i].index != tp->inputs[k].index)
				continue;

			f64 dist_i = tp->distances[base + itr - 1];
			f64 dist_k = tp->distances[base_k + itr - 1];

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
	for (size_t i = 0; i < size; i++) {
		if (tp->inputs[i].index != -2)
			continue;

		size_t index = i * size + itr;
		TouchInput last = tp->last[tp->indices[index]];

		update_from_last(tp, &tp->inputs[i], last);
		duplicates--;

		if (duplicates == 0)
			break;
	}

	return true;
}

void iptsd_finger_track(TouchProcessor *tp, size_t count)
{
	size_t size = std::size(tp->inputs);

	/*
	 * For every current input, we calculate the distance to all
	 * previous inputs. Then we use these distances to create a sorted
	 * list of their indices, going from nearest to furthest.
	 */
	for (size_t i = 0; i < size; i++) {
		size_t base = i * size;

		for (size_t j = 0; j < size; j++) {
			tp->indices[base + j] = j;

			TouchInput current = tp->inputs[i];
			TouchInput last = tp->last[j];

			if (current.index == -1 || last.index == -1) {
				tp->distances[base + j] = (f64)(1 << 30) + j;
				continue;
			}

			tp->distances[base + j] = current.dist(last);
		}

		/* Sort the list */
		bool swapped = true;

		for (size_t n = size; swapped; n--) {
			swapped = false;

			for (size_t k = 1; k < n; k++) {
				size_t index_a = tp->indices[base + k];
				size_t index_b = tp->indices[base + k - 1];

				f64 dist_a = tp->distances[base + index_a];
				f64 dist_b = tp->distances[base + index_b];

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
	for (size_t i = 0; i < count; i++) {
		size_t index = i * size;
		TouchInput last = tp->last[tp->indices[index]];

		update_from_last(tp, &tp->inputs[i], last);
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
	for (size_t j = 1; j < size; j++) {
		if (!find_duplicates(tp, count, j))
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
	for (size_t i = 0; i < size; i++) {
		if (tp->inputs[i].index == -1)
			continue;

		tp->inputs[i].slot = tp->inputs[i].index;
		tp->free_indices[tp->inputs[i].index] = false;
	}

	for (size_t i = 0; i < size; i++) {
		if (tp->inputs[i].index != -1)
			continue;

		for (size_t k = 0; k < size; k++) {
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
