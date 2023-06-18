// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_USAGE_HPP
#define IPTSD_HID_USAGE_HPP

#include <common/types.hpp>

namespace iptsd::hid {

struct Usage {
	u16 page;
	u16 value;
};

} // namespace iptsd::hid

#endif // IPTSD_HID_USAGE_HPP
