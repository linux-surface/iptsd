/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_FINGER_H_
#define _IPTSD_FINGER_H_

#include "touch-processing.h"

#define CONTACT_STABILITY_THRESHOLD 0.1

void iptsd_finger_track(struct iptsd_touch_processor *tp, int count);

#endif /* _IPTSD_FINGER_H_ */

