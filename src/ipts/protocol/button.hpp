// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_BUTTON_HPP
#define IPTSD_IPTS_PROTOCOL_BUTTON_HPP

#include <common/types.hpp>

#include <array>

namespace iptsd::ipts::protocol::button {

/*
 * This is an educated guess. Adjust if necessary.
 */
constexpr u16 MAX_PRESSURE = 1024;

/*!
 * A sample that describes the state of the (touchpad) button.
 *
 * Button reports signal clicks on an IPTS touchpad. One report can contain multiple samples,
 * chained together without any header. So far, all known data only contains two samples, but
 * to make sure, the payload size of the report should be used to determine the true number.
 */
struct [[gnu::packed]] Sample {
	//! How hard the user is pressing on the touchpad.
	u16 pressure;

	//! ...
	std::array<u8, 12> reserved1;

	//! Whether the user is pressing hard enough to trigger a button press.
	bool button;

	//! ...
	u8 reserved2;
};
static_assert(sizeof(Sample) == 16);

} // namespace iptsd::ipts::protocol::button

#endif // IPTSD_IPTS_PROTOCOL_BUTTON_HPP
