// SPDX-License-Identifier: GPL-2.0-or-later

#include <math.h>
#include <string.h>

#include "contact.h"
#include "heatmap.h"

static void cluster_add(struct cluster *c, int x, int y, int w)
{
	c->x += w * x;
	c->y += w * y;
	c->xx += w * x * x;
	c->yy += w * y * y;
	c->xy += w * x * y;
	c->w += w;

	if (c->max_v < w)
		c->max_v = w;
}

static void cluster_mean(struct cluster c, float *x, float *y)
{
	float fx = (float)c.x;
	float fy = (float)c.y;
	float fw = (float)c.w;

	*x = fx / fw;
	*y = fy / fw;
}

static void cluster_cov(struct cluster c, float *r1, float *r2, float *r3)
{
	float fx = (float)c.x;
	float fy = (float)c.y;
	float fxx = (float)c.xx;
	float fyy = (float)c.yy;
	float fxy = (float)c.xy;
	float fw = (float)c.w;

	*r1 = (fxx - (fx * fx / fw)) / fw;
	*r2 = (fyy - (fy * fy / fw)) / fw;
	*r3 = (fxy - (fx * fy / fw)) / fw;
}

static void __cluster_get(struct heatmap *hm, int x, int y, struct cluster *c)
{
	int v = heatmap_value(hm, x, y);

	if (!heatmap_is_touch(hm, x, y))
		return;

	if (heatmap_get_visited(hm, x, y))
		return;

	cluster_add(c, x, y, v);
	heatmap_set_visited(hm, x, y, true);

	__cluster_get(hm, x + 1, y, c);
	__cluster_get(hm, x - 1, y, c);
	__cluster_get(hm, x, y + 1, c);
	__cluster_get(hm, x, y - 1, c);
}

static struct cluster cluster_get(struct heatmap *hm, int x, int y)
{
	struct cluster c;

	memset(&c, 0, sizeof(struct cluster));
	__cluster_get(hm, x, y, &c);

	return c;
}

static struct contact contact_from_cluster(struct cluster cluster)
{
	struct contact c;

	float vx;
	float vy;
	float cv;

	cluster_mean(cluster, &c.x, &c.y);
	cluster_cov(cluster, &vx, &vy, &cv);
	float sqrtd = sqrtf((vx - vy) * (vx - vy) + 4 * cv * cv);

	c.ev1 = (vx + vy + sqrtd) / 2;
	c.ev2 = (vx + vy - sqrtd) / 2;

	c.qx1 = vx + cv - c.ev2;
	c.qy1 = vy + cv - c.ev2;
	c.qx2 = vx + cv - c.ev1;
	c.qy2 = vy + cv - c.ev1;

	float d1 = hypotf(c.qx1, c.qy1);
	float d2 = hypotf(c.qx2, c.qy2);

	c.qx1 /= d1;
	c.qy1 /= d1;
	c.qx2 /= d2;
	c.qy2 /= d2;

	c.max_v = cluster.max_v;
	c.is_palm = false;

	c.angle = M_PI_2 - atan2f(c.qy1, c.qx1);
	if (c.angle < 0)
		c.angle += M_PI;
	if (c.angle > M_PI)
		c.angle -= M_PI;

	return c;
}

static void contact_pca(struct contact c, float x, float y, float *rx, float *ry)
{
	*rx = c.qx1 * x + c.qx2 * y;
	*ry = c.qy1 * x + c.qy2 * y;
}

bool contact_near(struct contact c, struct contact other)
{
	float dx;
	float dy;

	contact_pca(other, c.x - other.x, c.y - other.y, &dx, &dy);

	dx = fabsf(dx);
	dy = fabsf(dy);

	dx /= 3.2 * sqrtf(other.ev1) + 8;
	dy /= 3.2 * sqrtf(other.ev2) + 8;

	return dx * dx + dy * dy <= 1;
}

int contacts_get(struct heatmap *hm, struct contact *contacts, int count)
{
	int c = 0;

	if (count == 0)
		return 0;

	for (int i = 0; i < hm->size; i++)
		hm->visited[i] = false;

	for (int x = 0; x < hm->width; x++) {
		for (int y = 0; y < hm->height; y++) {
			if (!heatmap_is_touch(hm, x, y))
				continue;

			if (heatmap_get_visited(hm, x, y))
				continue;

			struct cluster cluster = cluster_get(hm, x, y);
			contacts[c] = contact_from_cluster(cluster);

			// ignore 0 variance contacts
			if (contacts[c].ev2 > 0)
				c++;

			if (c == count)
				return c;
		}
	}

	return c;
}
