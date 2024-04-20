// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_REPORT_HPP
#define IPTSD_IPTS_PROTOCOL_REPORT_HPP

#include <common/types.hpp>

namespace iptsd::ipts::protocol::report {

enum class Type : u8 {
	HeatmapTimestamp = 0x0,
	HeatmapDimensions = 0x3,
	HeatmapData = 0x25,

	StylusMPP_1_0 = 0x10,
	StylusMPP_1_51 = 0x60,

	DftFrequencyNoise = 0x04,
	DftGeneral = 0x57,
	DftJnrOutput = 0x58,
	DftNoiseMetricsOutput = 0x59,
	DftDataSelection = 0x5A,
	DftMagnitude = 0x5B,
	DftWindow = 0x5C,
	DftMultipleRegion = 0x5D,
	DftTouchedAntennas = 0x5E,
	DftMetadata = 0x5F,
	DftDetection = 0x62,
	DftLift = 0x63,

	Button = 0x90,
};

/*!
 * Report frames contain very specific data, such as stylus coordinates or capacitive heatmaps.
 * Functionally they are equivalent to HID frames and legacy report groups, but in practice they
 * are different for ... reasons.
 *
 * A report frame consists of this header, followed by a payload.
 */
struct [[gnu::packed]] Frame {
	//! The type of the report frame. Determines the structure of the payload.
	Type type;

	//! ...
	u8 flags;

	//! The size of the payload, in bytes.
	u16 size;
};
static_assert(sizeof(Frame) == 4);

} // namespace iptsd::ipts::protocol::report

#endif // IPTSD_IPTS_PROTOCOL_REPORT_HPP
