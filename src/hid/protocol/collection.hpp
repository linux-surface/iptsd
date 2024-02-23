// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_PROTOCOL_COLLECTION_HPP
#define IPTSD_HID_PROTOCOL_COLLECTION_HPP

#include <common/types.hpp>

namespace iptsd::hid::protocol::collection {

enum class Type : u8 {
	Physical = 0x0,
	Application = 0x1,
	Logical = 0x2,
	Report = 0x3,
	NamedArray = 0x4,
	UsageSwitch = 0x5,
	UsageModifier = 0x6,

	Reserved = 0x7,
	Vendor = 0x80,
};

inline Type parse_type(const u8 value)
{
	if (value >= 0x7 && value <= 0x7F)
		return Type::Reserved;

	if (value >= 0x80)
		return Type::Vendor;

	return gsl::narrow<Type>(value);
}

} // namespace iptsd::hid::protocol::collection

#endif // IPTSD_HID_PROTOCOL_COLLECTION_HPP
