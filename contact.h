/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONTACT_H_
#define _IPTSD_CONTACT_H_

#include <stdbool.h>

#include "heatmap.h"

#define CONTACT_TOUCH_THRESHOLD 10

struct cluster {
	long x;
	long y;
	long xx;
	long yy;
	long xy;
	long w;
	long max_v;
};

struct contact {
	/* center */
	float x;
	float y;

	/* covariance matrix eigenvalues */
	float ev1;
	float ev2;

	/* covariance matrix eigenvectors */
	float qx1;
	float qy1;
	float qx2;
	float qy2;

	int max_v;
	bool is_palm;
};

int contacts_get(struct heatmap *hm, struct contact *contacts, int count);
void contacts_get_palms(struct contact *contacts, int count);

#endif /* _IPTSD_CONTACT_H_ */

