/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_HEATMAP_H_
#define _IPTSD_HEATMAP_H_

#include <stdbool.h>
#include <stdint.h>

struct heatmap {
	int width;
	int height;
	int size;
	int touch_threshold;
	float diagonal;

	uint8_t *data;
	bool *visited;
};

float heatmap_average(struct heatmap *hm);
uint8_t heatmap_value(struct heatmap *hm, int x, int y);
bool heatmap_is_touch(struct heatmap *hm, int x, int y);
bool heatmap_compare(struct heatmap *hm, int x1, int y1, int x2, int y2);
bool heatmap_get_visited(struct heatmap *hm, int x, int y);
void heatmap_set_visited(struct heatmap *hm, int x, int y, bool value);
int heatmap_init(struct heatmap *hm);
void heatmap_free(struct heatmap *hm);

#endif /* _IPTSD_HEATMAP_H_ */
