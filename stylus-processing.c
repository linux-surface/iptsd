// SPDX-License-Identifier: GPL-2.0-or-not

#include <math.h>

#include "stylus-processing.h"

void iptsd_stylus_processing_flush(struct iptsd_stylus_processor *sp)
{
	for (int i = 0; i < IPTSD_STYLUS_CACHED_FRAMES; i++) {
		sp->x_cache[i] = -1;
		sp->y_cache[i] = -1;
	}
}

void iptsd_stylus_processing_smooth(struct iptsd_stylus_processor *sp,
		int x, int y, int *sx, int *sy)
{
	sp->x_cache[0] = sp->x_cache[1];
	sp->x_cache[1] = sp->x_cache[2];
	sp->x_cache[2] = sp->x_cache[3];
	sp->x_cache[3] = sp->x_cache[4];
	sp->x_cache[4] = x;

	sp->y_cache[0] = sp->y_cache[1];
	sp->y_cache[1] = sp->y_cache[2];
	sp->y_cache[2] = sp->y_cache[3];
	sp->y_cache[3] = sp->y_cache[4];
	sp->y_cache[4] = y;

	*sx = 0;
	*sy = 0;

	int j = 0;

	for (int i = 0; i < IPTSD_STYLUS_CACHED_FRAMES; i++) {
		if (sp->x_cache[i] == -1 || sp->y_cache[i] == -1) {
			j++;
			continue;
		}

		*sx += sp->x_cache[i];
		*sy += sp->y_cache[i];
	}

	*sx /= IPTSD_STYLUS_CACHED_FRAMES - j;
	*sy /= IPTSD_STYLUS_CACHED_FRAMES - j;
}

void iptsd_stylus_processing_tilt(struct iptsd_stylus_processor *sp,
		int altitude, int azimuth, int *tx, int *ty)
{
	*tx = 0;
	*ty = 0;

	if (altitude <= 0)
		return;

	double alt = ((double)altitude) / 18000 * M_PI;
	double azm = ((double)azimuth) / 18000 * M_PI;

	double sin_alt = sin(alt);
	double sin_azm = sin(azm);

	double cos_alt = cos(alt);
	double cos_azm = cos(azm);

	double atan_x = atan2(cos_alt, sin_alt * cos_azm);
	double atan_y = atan2(cos_alt, sin_alt * sin_azm);

	*tx = 9000 - (atan_x * 4500 / M_PI_4);
	*ty = (atan_y * 4500 / M_PI_4) - 9000;
}

