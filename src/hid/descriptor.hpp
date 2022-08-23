/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_HID_DESCRIPTOR_HPP
#define IPTSD_HID_DESCRIPTOR_HPP

#include <common/types.hpp>
#include <hid/shim/hidrd.h>

#include <cstddef>
#include <gsl/gsl>
#include <vector>

namespace iptsd::hid {

class Descriptor {
public:
	void load(gsl::span<u8> raw);
	[[nodiscard]] const std::vector<const hidrd_item *> &items() const;

	[[nodiscard]] std::vector<u8> reports(hidrd_item_main_tag type) const;
	[[nodiscard]] std::vector<hidrd_usage> usage(u8 report) const;
	[[nodiscard]] hidrd_usage_page usage_page(u8 report) const;
	[[nodiscard]] u64 size(u8 report) const;

private:
	std::vector<u8> descriptor {};
	std::vector<const hidrd_item *> parsed {};
};

} /* namespace iptsd::hid */

#endif /* IPTSD_HID_DESCRIPTOR_HPP */
