/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_IPTS_PROTOCOL_HPP
#define IPTSD_IPTS_PROTOCOL_HPP

#include <common/types.hpp>

#include <array>

/* clang-format off */

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

constexpr u8 IPTS_RAW_FRAME_TYPE_STYLUS   = 0x6;
constexpr u8 IPTS_RAW_FRAME_TYPE_HEATMAP  = 0x8;

constexpr u8 IPTS_HID_FRAME_TYPE_HID      = 0x0;
constexpr u8 IPTS_HID_FRAME_TYPE_HEATMAP  = 0x1;
constexpr u8 IPTS_HID_FRAME_TYPE_METADATA = 0x2;
constexpr u8 IPTS_HID_FRAME_TYPE_RAW      = 0xEE;
constexpr u8 IPTS_HID_FRAME_TYPE_REPORTS  = 0xFF;

constexpr u8 IPTS_REPORT_TYPE_TIMESTAMP                = 0x00;
constexpr u8 IPTS_REPORT_TYPE_DIMENSIONS               = 0x03;
constexpr u8 IPTS_REPORT_TYPE_HEATMAP                  = 0x25;
constexpr u8 IPTS_REPORT_TYPE_STYLUS_V1                = 0x10;
constexpr u8 IPTS_REPORT_TYPE_STYLUS_V2                = 0x60;
constexpr u8 IPTS_REPORT_TYPE_FREQUENCY_NOISE          = 0x04;
constexpr u8 IPTS_REPORT_TYPE_PEN_GENERAL              = 0x57;
constexpr u8 IPTS_REPORT_TYPE_PEN_JNR_OUTPUT           = 0x58;
constexpr u8 IPTS_REPORT_TYPE_PEN_NOISE_METRICS_OUTPUT = 0x59;
constexpr u8 IPTS_REPORT_TYPE_PEN_DATA_SELECTION       = 0x5a;
constexpr u8 IPTS_REPORT_TYPE_PEN_MAGNITUDE            = 0x5b;
constexpr u8 IPTS_REPORT_TYPE_PEN_DFT_WINDOW           = 0x5c;
constexpr u8 IPTS_REPORT_TYPE_PEN_MULTIPLE_REGION      = 0x5d;
constexpr u8 IPTS_REPORT_TYPE_PEN_TOUCHED_ANTENNAS     = 0x5e;
constexpr u8 IPTS_REPORT_TYPE_PEN_METADATA             = 0x5f;
constexpr u8 IPTS_REPORT_TYPE_PEN_DETECTION            = 0x62;
constexpr u8 IPTS_REPORT_TYPE_PEN_LIFT                 = 0x63;

constexpr u8 IPTS_STYLUS_REPORT_MODE_BIT_PROXIMITY = 0;
constexpr u8 IPTS_STYLUS_REPORT_MODE_BIT_CONTACT   = 1;
constexpr u8 IPTS_STYLUS_REPORT_MODE_BIT_BUTTON    = 2;
constexpr u8 IPTS_STYLUS_REPORT_MODE_BIT_RUBBER    = 3;

constexpr u8 IPTS_DFT_NUM_COMPONENTS = 9;
constexpr u8 IPTS_DFT_MAX_ROWS       = 16;
constexpr u8 IPTS_DFT_PRESSURE_ROWS  = 6;

constexpr u8 IPTS_DFT_ID_POSITION = 6;
constexpr u8 IPTS_DFT_ID_BUTTON   = 9;
constexpr u8 IPTS_DFT_ID_PRESSURE = 11;

/*
 * Static limits for the data that is returned by IPTS
 */
constexpr u16 IPTS_MAX_X        = 9600;
constexpr u16 IPTS_MAX_Y        = 7200;
constexpr u16 IPTS_MAX_PRESSURE = 4096;
constexpr u8  IPTS_MAX_CONTACTS = 16;

/*
 * sqrt(IPTS_MAX_X² + IPTS_MAX_Y²)
 */
constexpr u32 IPTS_DIAGONAL = 12000;

/* clang-format on */

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
	u8 type;
	u8 flags;
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

struct [[gnu::packed]] ipts_timestamp {
	u8 reserved[2]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
	u16 count;
	u32 timestamp;
};

struct [[gnu::packed]] ipts_heatmap_header {
	u8 reserved[5]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
	u32 size;
};

struct [[gnu::packed]] ipts_pen_dft_window {
	u32 timestamp; // counting at approx 8MHz
	u8 num_rows;
	u8 seq_num;
	u8 reserved[3]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
	u8 data_type;
	u8 reserved2[2]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
};

struct [[gnu::packed]] ipts_pen_dft_window_row {
	u32 frequency;
	u32 magnitude;
	i16 real[IPTS_DFT_NUM_COMPONENTS]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
	i16 imag[IPTS_DFT_NUM_COMPONENTS]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
	i8 first;
	i8 last;
	i8 mid;
	i8 zero;
};

struct [[gnu::packed]] ipts_touch_metadata_size {
	u32 rows;
	u32 columns;
	u32 width;
	u32 height;
};

struct [[gnu::packed]] ipts_touch_metadata_transform {
	f32 xx, yx, tx;
	f32 xy, yy, ty;
};

struct [[gnu::packed]] ipts_touch_metadata_unknown {
	f32 unknown[16]; // NOLINT(modernize-avoid-c-arrays,cppcoreguidelines-avoid-c-arrays)
};

#endif /* IPTSD_IPTS_PROTOCOL_HPP */
