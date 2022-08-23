/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_IPTS_DEVICE_HPP
#define IPTSD_IPTS_DEVICE_HPP

#include <common/types.hpp>
#include <hid/device.hpp>

#include <gsl/gsl>
#include <linux/hidraw.h>
#include <string>
#include <utility>
#include <vector>

namespace iptsd::ipts {

class Device : public hid::Device {
private:
	bool is_set_mode(u8 report);
	u8 get_set_mode();

public:
	Device(const std::string &path) : hid::Device(path) {};

	bool is_touch_data(u8 report);
	std::size_t buffer_size();
	void set_mode(bool multitouch);
};

} // namespace iptsd::ipts

#endif /* IPTSD_IPTS_DEVICE_HPP */
