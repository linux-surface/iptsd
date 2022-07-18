/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_IPTS_PROTOCOL_HPP
#define IPTSD_IPTS_PROTOCOL_HPP

#include <common/types.hpp>

#include <array>

/*
 * If a report contains both of these usages, the report is used to send touch data
 */
constexpr u8 IPTS_HID_REPORT_USAGE_SCAN_TIME = 0x56;
constexpr u8 IPTS_HID_REPORT_USAGE_GESTURE_DATA = 0x61;

/*
 * The feature report that is used for switching into multitouch mode and back
 */
constexpr u8 IPTS_HID_FEATURE_REPORT_MODE = 0x5;

constexpr u8 IPTS_RAW_FRAME_TYPE_STYLUS = 0x6;
constexpr u8 IPTS_RAW_FRAME_TYPE_HEATMAP = 0x8;

constexpr u8 IPTS_HID_FRAME_TYPE_HID = 0x0;
constexpr u8 IPTS_HID_FRAME_TYPE_HEATMAP = 0x1;
constexpr u8 IPTS_HID_FRAME_TYPE_RAW = 0xEE;
constexpr u8 IPTS_HID_FRAME_TYPE_REPORTS = 0xFF;

constexpr u16 IPTS_REPORT_TYPE_HEATMAP_TIMESTAMP = 0x400;
constexpr u16 IPTS_REPORT_TYPE_HEATMAP_DIM = 0x403;
constexpr u16 IPTS_REPORT_TYPE_HEATMAP = 0x425;
constexpr u16 IPTS_REPORT_TYPE_STYLUS_V1 = 0x410;
constexpr u16 IPTS_REPORT_TYPE_STYLUS_V2 = 0x460;

constexpr u8 IPTS_STYLUS_REPORT_MODE_BIT_PROXIMITY = 0;
constexpr u8 IPTS_STYLUS_REPORT_MODE_BIT_CONTACT = 1;
constexpr u8 IPTS_STYLUS_REPORT_MODE_BIT_BUTTON = 2;
constexpr u8 IPTS_STYLUS_REPORT_MODE_BIT_RUBBER = 3;

/*
 * Static limits for the data that is returned by IPTS
 */
constexpr u16 IPTS_MAX_X = 9600;
constexpr u16 IPTS_MAX_Y = 7200;
constexpr u16 IPTS_MAX_PRESSURE = 4096;
constexpr u8 IPTS_MAX_CONTACTS = 16;

/*
 * sqrt(IPTS_MAX_X² + IPTS_MAX_Y²)
 */
constexpr u32 IPTS_DIAGONAL = 12000;

struct [[gnu::packed]] ipts_header {
	u8 report;
	u16 timestamp;
};

struct [[gnu::packed]] ipts_raw_header {
	u32 counter;
	u32 frames;
	u8 reserved[4]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
};

struct [[gnu::packed]] ipts_raw_frame {
	u16 index;
	u16 type;
	u32 size;
	u8 reserved[8]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
};

struct [[gnu::packed]] ipts_hid_frame {
	u32 size;
	u8 reserved1;
	u8 type;
	u8 reserved2;
};

struct [[gnu::packed]] ipts_report {
	u16 type;
	u16 size;
};

struct [[gnu::packed]] ipts_stylus_report {
	u8 elements;
	u8 reserved[3]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
	u32 serial;
};

struct [[gnu::packed]] ipts_stylus_data_v2 {
	u16 timestamp;
	u16 mode;
	u16 x;
	u16 y;
	u16 pressure;
	u16 altitude;
	u16 azimuth;
	u8 reserved[2]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
};

struct [[gnu::packed]] ipts_stylus_data_v1 {
	u8 reserved[4]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
	u8 mode;
	u16 x;
	u16 y;
	u16 pressure;
	u8 reserved2;
};

struct [[gnu::packed]] ipts_heatmap_dim {
	u8 height;
	u8 width;
	u8 y_min;
	u8 y_max;
	u8 x_min;
	u8 x_max;
	u8 z_min;
	u8 z_max;
};

struct [[gnu::packed]] ipts_heatmap_timestamp {
	u8 reserved[2]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
	u16 count;
	u32 timestamp;
};

struct [[gnu::packed]] ipts_heatmap_header {
	u8 reserved[5]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
	u32 size;
};

#endif /* IPTSD_IPTS_PROTOCOL_HPP */
