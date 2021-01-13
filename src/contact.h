/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONTACT_H_
#define _IPTSD_CONTACT_H_

#include <stdbool.h>

#include "constants.h"
#include "heatmap.h"

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

	float angle;

	int max_v;
	bool is_palm;
};

int contacts_get(struct heatmap *hm, struct contact *contacts, int count);
bool contact_near(struct contact c, struct contact other);

#endif /* _IPTSD_CONTACT_H_ */
