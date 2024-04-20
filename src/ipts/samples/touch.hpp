// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_SAMPLES_TOUCH_HPP
#define IPTSD_IPTS_SAMPLES_TOUCH_HPP

#include <common/types.hpp>

#include <gsl/gsl>

namespace iptsd::ipts::samples {

struct Touch {
	//! How many rows there are in the heatmap.
	u8 rows = 0;

	//! How many columns there are in the heatmap.
	u8 columns = 0;

	//! The lowest value that can occur in the heatmap.
	u8 min = 0;

	//! The largest value that can occur in the heatmap.
	u8 max = 0;

	//! The capacitive heatmap, layed out in row-major mode.
	gsl::span<u8> heatmap {};
};

} // namespace iptsd::ipts::samples

#endif // IPTSD_IPTS_SAMPLES_TOUCH_HPP
