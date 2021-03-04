/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_IPTS_CONTROL_HPP_
#define _IPTSD_IPTS_CONTROL_HPP_

#include "ipts.h"

#include <common/types.hpp>

#include <array>
#include <cstddef>

class IptsControl {
public:
	struct ipts_device_info info;
	u32 current_doorbell;

	IptsControl();
	~IptsControl();

	void send_feedback();
	u32 doorbell();
	ssize_t read(void *buf, size_t size);
	void reset();

private:
	std::array<int, IPTS_BUFFERS> files;

	int current();
	bool ready();
	void wait_for_device();
	void get_device_info();
	void send_feedback(int file);
	void flush();
};

#endif /* _IPTSD_IPTS_CONTROL_HPP_ */
