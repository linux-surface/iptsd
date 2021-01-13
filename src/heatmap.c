// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "heatmap.h"

float heatmap_average(struct heatmap *hm)
{
	float value = 0;

	for (int i = 0; i < hm->size; i++)
		value += hm->data[i];

	return value / hm->size;
}

uint8_t heatmap_value(struct heatmap *hm, int x, int y)
{
	if (x < 0 || x >= hm->width)
		return 0;

	if (y < 0 || y >= hm->height)
		return 0;

	return hm->data[y * hm->width + x];
}

bool heatmap_is_touch(struct heatmap *hm, int x, int y)
{
	return heatmap_value(hm, x, y) >= hm->touch_threshold;
}

bool heatmap_compare(struct heatmap *hm, int x1, int y1, int x2, int y2)
{
	int v1 = heatmap_value(hm, x1, y1);
	int v2 = heatmap_value(hm, x2, y2);

	if (v2 > v1)
		return false;

	if (v2 < v1)
		return true;

	if (x2 > x1)
		return false;

	if (x2 < x1)
		return true;

	if (y2 > y1)
		return false;

	if (y2 < y1)
		return true;

	return y2 == y1;
}

bool heatmap_get_visited(struct heatmap *hm, int x, int y)
{
	if (x < 0 || x >= hm->width)
		return false;

	if (y < 0 || y >= hm->height)
		return false;

	return hm->visited[y * hm->width + x];
}

void heatmap_set_visited(struct heatmap *hm, int x, int y, bool value)
{
	if (x < 0 || x >= hm->width)
		return;

	if (y < 0 || y >= hm->height)
		return;

	hm->visited[y * hm->width + x] = value;
}

void heatmap_free(struct heatmap *hm)
{
	if (hm->data)
		free(hm->data);

	if (hm->visited)
		free(hm->visited);
}

int heatmap_init(struct heatmap *hm)
{
	hm->size = hm->width * hm->height;
	hm->diagonal = sqrtf(hm->width * hm->width + hm->height * hm->height);

	hm->data = calloc(hm->size, sizeof(uint8_t));
	hm->visited = calloc(hm->size, sizeof(bool));

	if (!hm->data || !hm->visited) {
		heatmap_free(hm);
		return -ENOMEM;
	}

	return 0;
}
