/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_IPTS_PROTOCOL_H
#define IPTSD_IPTS_PROTOCOL_H

#include <stdint.h>

#define IPTS_HID_REPORT_USAGE_SCAN_TIME	   0x56
#define IPTS_HID_REPORT_USAGE_GESTURE_DATA 0x61

#define IPTS_HID_FEATURE_REPORT_MODE 0x5

#define IPTS_RAW_FRAME_TYPE_STYLUS  0x6
#define IPTS_RAW_FRAME_TYPE_HEATMAP 0x8

#define IPTS_HID_FRAME_TYPE_HID	    0x0
#define IPTS_HID_FRAME_TYPE_HEATMAP 0x1
#define IPTS_HID_FRAME_TYPE_RAW	    0xEE
#define IPTS_HID_FRAME_TYPE_REPORTS 0xFF

#define IPTS_REPORT_TYPE_HEATMAP_TIMESTAMP 0x400
#define IPTS_REPORT_TYPE_HEATMAP_DIM	   0x403
#define IPTS_REPORT_TYPE_HEATMAP	   0x425
#define IPTS_REPORT_TYPE_STYLUS_V1	   0x410
#define IPTS_REPORT_TYPE_STYLUS_V2	   0x460

#define IPTS_STYLUS_REPORT_MODE_BIT_PROXIMITY 0
#define IPTS_STYLUS_REPORT_MODE_BIT_CONTACT   1
#define IPTS_STYLUS_REPORT_MODE_BIT_BUTTON    2
#define IPTS_STYLUS_REPORT_MODE_BIT_RUBBER    3

#define IPTS_MAX_X    9600
#define IPTS_MAX_Y    7200
#define IPTS_DIAGONAL 12000

#define IPTS_MAX_CONTACTS 16

struct ipts_header {
	uint8_t report;
	uint16_t timestamp;
} __attribute__((__packed__));

struct ipts_raw_header {
	uint32_t counter;
	uint32_t frames;
	uint8_t reserved[4];
} __attribute__((__packed__));

struct ipts_raw_frame {
	uint16_t index;
	uint16_t type;
	uint32_t size;
	uint8_t reserved[8];
} __attribute__((__packed__));

struct ipts_hid_frame {
	uint32_t size;
	uint8_t reserved1;
	uint8_t type;
	uint8_t reserved2;
} __attribute__((__packed__));

struct ipts_report {
	uint16_t type;
	uint16_t size;
} __attribute__((__packed__));

struct ipts_stylus_report {
	uint8_t elements;
	uint8_t reserved[3];
	uint32_t serial;
} __attribute__((__packed__));

struct ipts_stylus_data_v2 {
	uint16_t timestamp;
	uint16_t mode;
	uint16_t x;
	uint16_t y;
	uint16_t pressure;
	uint16_t altitude;
	uint16_t azimuth;
	uint8_t reserved[2];
} __attribute__((__packed__));

struct ipts_stylus_data_v1 {
	uint8_t reserved[4];
	uint8_t mode;
	uint16_t x;
	uint16_t y;
	uint16_t pressure;
	uint8_t reserved2;
} __attribute__((__packed__));

struct ipts_heatmap_dim {
	uint8_t height;
	uint8_t width;
	uint8_t y_min;
	uint8_t y_max;
	uint8_t x_min;
	uint8_t x_max;
	uint8_t z_min;
	uint8_t z_max;
} __attribute__((__packed__));

struct ipts_heatmap_timestamp {
	uint8_t reserved[2];
	uint16_t count;
	uint32_t timestamp;
} __attribute__((__packed__));

struct ipts_heatmap_header {
	uint8_t reserved[5];
	uint32_t size;
} __attribute__((__packed__));

#endif /* IPTSD_IPTS_PROTOCOL_H */
