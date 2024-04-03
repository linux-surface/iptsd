// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_TOUCHPAD_HPP
#define IPTSD_IPTS_PROTOCOL_TOUCHPAD_HPP

#include <common/types.hpp>

#include <array>

namespace iptsd::ipts::protocol::touchpad {

/*
 * This is an educated guess. Adjust if necessary.
 */
constexpr u16 MAX_PRESSURE = 1024;

/*!
 * A sample that describes the state of the touchpad.
 *
 * Instead of having separate physical buttons for left and right click, the touchpad is built
 * as a single large button. The driver is expected to use the heatmap data to determine which side
 * of the button got pressed.
 *
 * A touchpad button report can contain multiple samples, chained together without any header.
 * The payload size of the report should be used to determined how many samples there are.
 */
struct [[gnu::packed]] Sample {
	//! How hard the user is pressing their finger on the touchpad.
	u16 pressure;

	//! ...
	std::array<u8, 12> reserved1;

	//! Whether the user is pressing hard enough to trigger a button press.
	bool button;

	//! ...
	bool reserved2;
};
static_assert(sizeof(Sample) == 16);

} // namespace iptsd::ipts::protocol::touchpad

#endif // IPTSD_IPTS_PROTOCOL_TOUCHPAD_HPP
