// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_SPEC_HPP
#define IPTSD_HID_SPEC_HPP

#include <common/types.hpp>

namespace iptsd::hid {

constexpr static u8 BITS_TAG = 0b11110000;
constexpr static u8 BITS_TYPE = 0b00001100;
constexpr static u8 BITS_SIZE = 0b00000011;

constexpr static u8 SHIFT_TAG = 4;
constexpr static u8 SHIFT_TYPE = 2;
constexpr static u8 SHIFT_SIZE = 0;

/*
 * 6.2.2.2 Short Items
 */
enum class ItemType : u8 {
	Main = 0,
	Global = 1,
	Local = 2,
	Reserved = 3,
};

/*
 * 6.2.2.4 Main Items
 */
enum class TagMain : u8 {
	Input = 0b1000,
	Output = 0b1001,
	Feature = 0b1011,
	Collection = 0b1010,
	EndCollection = 0b1100,
};

enum class TagGlobal : u8 {
	UsagePage = 0b0000,
	ReportSize = 0b0111,
	ReportId = 0b1000,
	ReportCount = 0b1001,
};

enum class TagLocal : u8 {
	Usage = 0b0000,
	UsageMinimum = 0b0001,
	UsageMaximum = 0b0010,
};

} // namespace iptsd::hid

#endif // IPTSD_HID_SPEC_HPP
