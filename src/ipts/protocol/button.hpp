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

/*!
 * The state of the button on a touchpad that has no moving parts and measures force instead.
 *
 * Seen on the Surface Laptop 7 15" (045E:0C77), which sends one of these with every heatmap
 * instead of the button report above. The device decides by itself when the force amounts
 * to a click, with a higher threshold for pressing than for releasing.
 */
struct [[gnu::packed]] ForceSample {
	//! How hard the user is pressing on the touchpad.
	u16 pressure;

	//! ...
	u16 reserved1;

	//! Six floating point values that rise with the force. Probably the individual sensors.
	std::array<f32, 6> reserved2;

	//! Whether any force is measured.
	bool contact;

	//! Whether the user is pressing hard enough to trigger a button press.
	bool button;

	//! How hard the user is pressing on the touchpad (follows @ref pressure closely).
	u16 pressure2;

	//! The force above which the button counts as pressed.
	u16 press_threshold;

	//! The force below which the button counts as released.
	u16 release_threshold;

	//! ...
	std::array<u8, 12> reserved3;
};
static_assert(sizeof(ForceSample) == 48);

} // namespace iptsd::ipts::protocol::button

#endif // IPTSD_IPTS_PROTOCOL_BUTTON_HPP
