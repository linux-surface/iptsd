// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_METADATA_HPP
#define IPTSD_IPTS_METADATA_HPP

#include <common/types.hpp>

#include <gsl/gsl>

namespace iptsd::ipts {

struct Metadata {
	//! How many rows the heatmaps sent by the device will have.
	u8 rows = 0;

	//! How many columns the heatmaps sent by the device will have.
	u8 columns = 0;

	//! The width of the screen in centimeters.
	f64 width = 0;

	//! The height of the screen in centimeters.
	f64 height = 0;

	//! Whether the heatmaps are inverted on the X / horizontal axis.
	bool invert_x = false;

	//! Whether the heatmaps are inverted on the Y / vertical axis.
	bool invert_y = false;
};

} // namespace iptsd::ipts

#endif // IPTSD_IPTS_METADATA_HPP
