/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_IPTS_DEVICE_HPP
#define IPTSD_IPTS_DEVICE_HPP

#include "parser.hpp"

#include <common/types.hpp>
#include <hid/device.hpp>

#include <gsl/gsl>
#include <linux/hidraw.h>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace iptsd::ipts {

class Device : public hid::Device {
private:
	[[nodiscard]] bool is_set_mode(u8 report) const;
	[[nodiscard]] u8 get_set_mode() const;
	[[nodiscard]] bool is_metadata_report(u8 report) const;
	[[nodiscard]] u8 get_metadata_report_id() const;

public:
	Device(const std::string &path) : hid::Device(path) {};

	void set_mode(bool multitouch) const;
	[[nodiscard]] bool is_touch_data(u8 report) const;
	[[nodiscard]] std::size_t buffer_size() const;
	[[nodiscard]] std::optional<const Metadata> get_metadata() const;
};

} // namespace iptsd::ipts

#endif /* IPTSD_IPTS_DEVICE_HPP */
