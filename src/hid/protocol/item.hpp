// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_PROTOCOL_ITEM_HPP
#define IPTSD_HID_PROTOCOL_ITEM_HPP

#include <common/casts.hpp>
#include <common/types.hpp>

namespace iptsd::hid::protocol::item {

enum class Size : u8 {
	NoPayload = 0b00,
	OneByte = 0b01,
	TwoBytes = 0b10,
	FourBytes = 0b11,
};

enum class Type : u8 {
	Main = 0b00,
	Global = 0b01,
	Local = 0b10,
	Reserved = 0b11,
};

enum class Tag : u8 {
	// Main
	Input = 0b100000,
	Output = 0b100100,
	Feature = 0b101100,
	Collection = 0b101000,
	EndCollection = 0b110000,

	// Global
	UsagePage = 0b000001,
	LocalMinimum = 0b000101,
	LocalMaximum = 0b001001,
	PhysicalMinimum = 0b001101,
	PhysicalMaximum = 0b010001,
	UnitExponent = 0b010101,
	Unit = 0b011001,
	ReportSize = 0b011101,
	ReportID = 0b100001,
	ReportCount = 0b100101,
	Push = 0b101001,
	Pop = 0b101101,

	// Local
	Usage = 0b000010,
	UsageMinimum = 0b000110,
	UsageMaximum = 0b001010,
	DesignatorIndex = 0b001110,
	DesignatorMinimum = 0b010010,
	DesignatorMaximum = 0b010110,
	StringIndex = 0b011110,
	StringMinimum = 0b100010,
	StringMaximum = 0b100110,
	Delimiter = 0b101010,
};

/*!
 * An item is a component of a report descriptor. It consists of this header, followed
 * by an optional payload.
 *
 * It provides a piece of information about the device and modifies the current state of the
 * report descriptor parser.
 */
struct [[gnu::packed]] Header {
	Size size : 2;
	Tag tag : 6;

	/*!
	 * Returns the type of the item.
	 *
	 * The type is contained inside the first two bits of the tag value.
	 * It could be split out into its own variable, but then the tag would
	 * require multiple enum types.
	 */
	[[nodiscard]] Type type() const
	{
		return gsl::narrow<Type>(gsl::narrow<u8>(this->tag) & 0b000011);
	}
};
static_assert(sizeof(Header) == 1);

} // namespace iptsd::hid::protocol::item

#endif // IPTSD_HID_PROTOCOL_ITEM_HPP
