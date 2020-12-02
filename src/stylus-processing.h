/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_STYLUS_PROCESSING_H_
#define _IPTSD_STYLUS_PROCESSING_H_

#include <stdint.h>

#include "protocol.h"

struct iptsd_stylus_processor {
	uint16_t timestamp;
	uint16_t mode;
	uint32_t x;
	uint32_t y;
	uint32_t pressure;
	uint32_t altitude;
	uint32_t azimuth;
	int count;
};

void iptsd_stylus_processing_add(struct iptsd_stylus_processor *proc,
		struct ipts_stylus_data data);
void iptsd_stylus_processing_get(struct iptsd_stylus_processor *proc,
		struct ipts_stylus_data *out);
void iptsd_stylus_processing_flush(struct iptsd_stylus_processor *proc);

#endif /* _IPTSD_STYLUS_PROCESSING_H_ */
