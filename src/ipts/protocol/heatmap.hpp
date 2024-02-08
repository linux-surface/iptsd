// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_HEATMAP_HPP
#define IPTSD_IPTS_PROTOCOL_HEATMAP_HPP

#include <common/types.hpp>

#include <array>

namespace iptsd::ipts::protocol::heatmap {

/*!
 * A heatmap frame consists of this header, followed by raw data.
 */
struct [[gnu::packed]] Frame {
	//! ...
	std::array<u8, 5> reserved;

	//! How many bytes of data are following this header.
	u32 size;
};
static_assert(sizeof(Frame) == 9);

/*!
 * Describes the size of a heatmap, as well as the range of values it can contain.
 */
struct [[gnu::packed]] Dimensions {
	//! How many rows there are in the heatmap.
	u8 rows;

	//! How many columns there are in the heatmap.
	u8 columns;

	//! The lowest valid Y coordinate in the heatmap.
	//! Should always be 0.
	u8 y_min;

	//! The largest valid Y coordinate in the heatmap.
	//! Should always be @ref rows - 1.
	u8 y_max;

	//! The lowest valid X coordinate in the heatmap.
	//! Should always be 0.
	u8 x_min;

	//! The largest valid X coordinate in the heatmap.
	//! Should always be @ref columns - 1.
	u8 x_max;

	//! The lowest value that a pixel in the heatmap can assume.
	u8 z_min;

	//! The highest value that a pixel in the heatmap can assume.
	//! On some devices, this is (incorrectly) set to 0.
	u8 z_max;
};
static_assert(sizeof(Dimensions) == 8);

} // namespace iptsd::ipts::protocol::heatmap

#endif // IPTSD_IPTS_PROTOCOL_HEATMAP_HPP
