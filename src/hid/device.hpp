/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_HID_DEVICE_HPP
#define IPTSD_HID_DEVICE_HPP

#include "descriptor.hpp"

#include <common/types.hpp>

#include <gsl/gsl>
#include <linux/hidraw.h>
#include <string>
#include <vector>

namespace iptsd::hid {

class Device {
public:
	Device(const std::string &path);

	[[nodiscard]] i16 product() const;
	[[nodiscard]] i16 vendor() const;

	[[nodiscard]] const Descriptor &descriptor() const;

	[[nodiscard]] ssize_t read(gsl::span<u8> buffer) const;
	void get_feature(gsl::span<u8> report) const;
	void set_feature(gsl::span<u8> report) const;

private:
	int device = -1;
	struct hidraw_devinfo devinfo {};
	Descriptor desc {};
};

} // namespace iptsd::hid

#endif /* IPTSD_HID_DEVICE_HPP */
