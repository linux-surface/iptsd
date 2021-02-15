/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_IPTS_CONTROL_HPP_
#define _IPTSD_IPTS_CONTROL_HPP_

#include "ipts.h"

#include <common/types.hpp>

#include <cstddef>

class IptsdControl {
public:
	struct ipts_device_info info;
	u32 current_doorbell;

	IptsdControl(void);
	~IptsdControl(void);

	void send_feedback(void);
	u32 doorbell(void);
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

#endif /* _IPTSD_IPTS_CONTROL_HPP_ */
