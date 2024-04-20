// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_SAMPLES_STYLUS_HPP
#define IPTSD_IPTS_SAMPLES_STYLUS_HPP

#include <common/types.hpp>

namespace iptsd::ipts::samples {

struct Stylus {
	//! Whether the stylus is near the screen.
	bool proximity = false;

	//! Whether the stylus is touching the screen.
	bool contact = false;

	//! Whether the side button of the stylus is being pressed.
	bool button = false;

	//! Whether the stylus is in eraser mode.
	bool rubber = false;

	//! The time at which this sample was generated.
	u16 timestamp = 0;

	//! The X / horizontal coordinate of the stylus tip.
	//! Range: 0 to 1
	f64 x = 0;

	//! The Y / vertical coordinate of the stylus tip.
	//! Range: 0 to 1
	f64 y = 0;

	//! How hard the stylus is being pressed onto the display.
	//! Range: 0 to 1
	f64 pressure = 0;

	//! The angle between the stylus tip and the display.
	//! Unit: Radians
	f64 altitude = 0;

	//! The direction in which the stylus tip is pointing.
	//! Unit: Radians
	f64 azimuth = 0;
};

} // namespace iptsd::ipts::samples

#endif // IPTSD_IPTS_SAMPLES_STYLUS_HPP
