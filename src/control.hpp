/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_CONTROL_HPP_
#define _IPTSD_CONTROL_HPP_

#include "ipts.h"

#include <cstddef>
#include <cstdint>

class IptsdControl {
public:
	struct ipts_device_info info;
	uint32_t current_doorbell;

	IptsdControl(void);
	~IptsdControl(void);

	void send_feedback(void);
	uint32_t doorbell(void);
	int read(void *buf, size_t size);
	void reset(void);

private:
	int files[IPTS_BUFFERS];

	int current(void);
	bool ready(void);
	void wait_for_device(void);
	void get_device_info(void);
	void send_feedback(int file);
	void flush(void);
};

#endif /* _IPTSD_CONTROL_HPP_ */
