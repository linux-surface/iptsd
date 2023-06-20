// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_DEVICE_HPP
#define IPTSD_HID_DEVICE_HPP

#include "report.hpp"

#include <common/types.hpp>

#include <gsl/gsl>

#include <functional>
#include <vector>

namespace iptsd::hid {

class Device {
public:
	virtual ~Device() = default;

	virtual u16 vendor() = 0;
	virtual u16 product() = 0;
	virtual const std::vector<Report> &descriptor() = 0;

	virtual isize read(gsl::span<u8> buffer) = 0;
	virtual void get_feature(gsl::span<u8> report) = 0;
	virtual void set_feature(gsl::span<u8> report) = 0;
};

} // namespace iptsd::hid

#endif // IPTSD_HID_DEVICE_HPP
