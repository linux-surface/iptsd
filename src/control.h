/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONTROL_H_
#define _IPTSD_CONTROL_H_

#include <stddef.h>
#include <stdint.h>

#include "ipts.h"

struct iptsd_control {
	int files[IPTS_BUFFERS];
	uint32_t current_doorbell;
	struct ipts_device_info device_info;
};

int iptsd_control_current_buffer(struct iptsd_control *control);
int iptsd_control_current_file(struct iptsd_control *control);
int iptsd_control_ready(struct iptsd_control *control);
void iptsd_control_wait_for_device(struct iptsd_control *control);
int iptsd_control_send_feedback(struct iptsd_control *control);
int iptsd_control_flush(struct iptsd_control *control);
int64_t iptsd_control_doorbell(struct iptsd_control *control);
int iptsd_control_device_info(struct iptsd_control *control);
int iptsd_control_start(struct iptsd_control *control);
int iptsd_control_read(struct iptsd_control *control, void *buf, size_t count);
int iptsd_control_stop(struct iptsd_control *control);
int iptsd_control_reset(struct iptsd_control *control);

#endif /* _IPTSD_CONTROL_H_ */
