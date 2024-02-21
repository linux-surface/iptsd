// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_HID_HPP
#define IPTSD_IPTS_PROTOCOL_HID_HPP

#include <common/types.hpp>

namespace iptsd::ipts::protocol::hid {

/*!
 * This header is prefixed to all data received from the device over HID.
 *
 * It is followed by a single HID frame that wraps the actual payload of the report.
 */
struct [[gnu::packed]] ReportHeader {
	u8 id;
	u16 timestamp;
};
static_assert(sizeof(ReportHeader) == 3);

enum class FrameType : u8 {
	//! The HID frame contains further HID frames, chained together.
	Hid = 0x0,

	//! The HID frame contains a heatmap frame.
	Heatmap = 0x1,

	//! The HID frame contains a metadata frame.
	//! These are only returned by a HID feature report and don't appear in normal data.
	Metadata = 0x2,

	//! The HID frame contains a legacy frame. See @ref protocol::legacy::Header
	//! NOTE: This is a made up type for transporting legacy frames over HID.
	Legacy = 0xEE,

	//! The HID frame contains a report frame.
	Reports = 0xFF,
};

/*!
 * HID frames are the top-level data structure in the data received from the device.
 *
 * They can roughly be thought of as categories, that don't contain specific data directly
 * but instead group it together.
 *
 * A HID frame consists of this header, followed by a payload.
 */
struct [[gnu::packed]] Frame {
	//! The size of the entire HID frame, so this header plus the size of the payload.
	u32 size;

	//! ...
	u8 reserved1;

	//! The type of the HID frame. Determines the structure of the payload.
	FrameType type;

	//! ...
	u8 reserved2;
};
static_assert(sizeof(Frame) == 7);

} // namespace iptsd::ipts::protocol::hid

#endif // IPTSD_IPTS_PROTOCOL_HID_HPP
