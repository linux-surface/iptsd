/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_STYLUS_PROCESSING_H_
#define _IPTSD_STYLUS_PROCESSING_H_

#define IPTSD_STYLUS_CACHED_FRAMES 5

struct iptsd_stylus_processor {
	int x_cache[IPTSD_STYLUS_CACHED_FRAMES];
	int y_cache[IPTSD_STYLUS_CACHED_FRAMES];
};

void iptsd_stylus_processing_flush(struct iptsd_stylus_processor *sp);
void iptsd_stylus_processing_smooth(struct iptsd_stylus_processor *sp,
		int x, int y, int *sx, int *sy);
void iptsd_stylus_processing_tilt(struct iptsd_stylus_processor *sp,
		int altitude, int azimuth, int *tx, int *ty);

#endif /* _IPTSD_STYLUS_PROCESSING_H_ */

