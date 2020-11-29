/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_PROTOCOL_H_
#define _IPTSD_PROTOCOL_H_

#include <stdint.h>

#define IPTS_DATA_TYPE_PAYLOAD      0x0
#define IPTS_DATA_TYPE_ERROR        0x1
#define IPTS_DATA_TYPE_VENDOR_DATA  0x2
#define IPTS_DATA_TYPE_HID_REPORT   0x3
#define IPTS_DATA_TYPE_GET_FEATURES 0x4

#define IPTS_PAYLOAD_FRAME_TYPE_STYLUS 0x6
#define IPTS_PAYLOAD_FRAME_TYPE_TOUCH  0x8

#define IPTS_REPORT_TYPE_TOUCH_HEATMAP_DIM  0x403
#define IPTS_REPORT_TYPE_TOUCH_HEATMAP      0x425
#define IPTS_REPORT_TYPE_STYLUS_NO_TILT     0x410
#define IPTS_REPORT_TYPE_STYLUS_TILT        0x461
#define IPTS_REPORT_TYPE_STYLUS_TILT_SERIAL 0x460

#define IPTS_STYLUS_REPORT_MODE_PROX   (1 << 0)
#define IPTS_STYLUS_REPORT_MODE_TOUCH  (1 << 1)
#define IPTS_STYLUS_REPORT_MODE_BUTTON (1 << 2)
#define IPTS_STYLUS_REPORT_MODE_RUBBER (1 << 3)

#define IPTS_SINGLETOUCH_REPORT_ID 0x40
#define IPTS_SINGLETOUCH_MAX_VALUE (1 << 15)

#define IPTS_MAX_X    9600
#define IPTS_MAX_Y    7200
#define IPTS_DIAGONAL 12000

struct ipts_data {
	uint32_t type;
	uint32_t size;
	uint32_t buffer;
	uint8_t reserved[52];
	uint8_t data[];
} __attribute__((__packed__));

struct ipts_payload {
	uint32_t counter;
	uint32_t frames;
	uint8_t reserved[4];
	uint8_t data[];
} __attribute__((__packed__));

struct ipts_payload_frame {
	uint16_t index;
	uint16_t type;
	uint32_t size;
	uint8_t reserved[8];
	uint8_t data[];
} __attribute__((__packed__));

struct ipts_report {
	uint16_t type;
	uint16_t size;
	uint8_t data[];
} __attribute__((__packed__));

struct ipts_stylus_report {
	uint8_t elements;
	uint8_t reserved[3];
	uint8_t data[];
} __attribute__((__packed__));

struct ipts_stylus_report_serial {
	uint8_t elements;
	uint8_t reserved[3];
	uint32_t serial;
	uint8_t data[];
} __attribute__((__packed__));

struct ipts_stylus_data {
	uint16_t timestamp;
	uint16_t mode;
	uint16_t x;
	uint16_t y;
	uint16_t pressure;
	uint16_t altitude;
	uint16_t azimuth;
	uint8_t reserved[2];
} __attribute__((__packed__));

struct ipts_stylus_data_no_tilt {
	uint8_t reserved[4];
	uint8_t mode;
	uint16_t x;
	uint16_t y;
	uint16_t pressure;
	uint8_t reserved2;
} __attribute__((__packed__));

struct ipts_singletouch_data {
	uint8_t touch;
	uint16_t x;
	uint16_t y;
} __attribute__((__packed__));

#endif /* _IPTSD_PROTOCOL_H_ */

