// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_METADATA_HPP
#define IPTSD_IPTS_PROTOCOL_METADATA_HPP

#include <common/types.hpp>

#include <array>

namespace iptsd::ipts::protocol::metadata {

/*!
 * The size of the physical touchscreen, as well as the heatmap data produced by it.
 */
struct [[gnu::packed]] Dimensions {
	//! How many rows the heatmaps sent by the device will have.
	//! Heatmaps are stored in row-major order.
	u32 rows;

	//! How many columns the heatmaps sent by the device will have.
	//! Heatmaps are stored in row-major order.
	u32 columns;

	//! The physical width of the touchscreen.
	//! Unit: millimeter * 100
	u32 width;

	//! The physical height of the touchscreen.
	//! Unit: millimeter * 100
	u32 height;
};
static_assert(sizeof(Dimensions) == 16);

/*!
 * Transform matrix that converts between rows/cols and physical screen coordinates.
 */
struct [[gnu::packed]] Transform {
	f32 xx, yx, tx;
	f32 xy, yy, ty;
};
static_assert(sizeof(Transform) == 24);

/*!
 * Unkown floats, possibly the tilt transform.
 */
struct [[gnu::packed]] Unknown {
	std::array<f32, 16> unknown;
};
static_assert(sizeof(Unknown) == 64);

} // namespace iptsd::ipts::protocol::metadata

#endif // IPTSD_IPTS_PROTOCOL_METADATA
