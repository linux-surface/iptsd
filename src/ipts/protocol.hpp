// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_HPP
#define IPTSD_IPTS_PROTOCOL_HPP

#include <common/types.hpp>

// clang-format off

constexpr u16 IPTS_HID_REPORT_USAGE_PAGE_DIGITIZER = 0x000D;
constexpr u16 IPTS_HID_REPORT_USAGE_PAGE_VENDOR    = 0xFF00;

/*
 * If a report contains both of these usages, the report is used to send touch data
 */
constexpr u8 IPTS_HID_REPORT_USAGE_SCAN_TIME = 0x56;
constexpr u8 IPTS_HID_REPORT_USAGE_GESTURE_DATA = 0x61;

/*
 * If a one byte feature report contains only this usage, it is used for switching modes.
 */
constexpr u8 IPTS_HID_REPORT_USAGE_SET_MODE = 0xC8;

/*
 * If a feature report contains only this usage, it contains touch/pen metadata.
 */
constexpr u8 IPTS_HID_REPORT_USAGE_METADATA = 0x63;

// clang-format on

struct [[gnu::packed]] ipts_dimensions {
	u8 height;
	u8 width;
	u8 y_min;
	u8 y_max;
	u8 x_min;
	u8 x_max;
	u8 z_min;
	u8 z_max;
};

struct [[gnu::packed]] ipts_heatmap_header {
	u8 reserved[5]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
	u32 size;
};

#endif // IPTSD_IPTS_PROTOCOL_HPP
