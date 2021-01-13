/* SPDX-License-Identifier: GPL-2.0-or-later WITH Linux-syscall-note */
/*
 * Copyright (c) 2016 Intel Corporation
 * Copyright (c) 2020 Dorian Stoll
 *
 * Linux driver for Intel Precise Touch & Stylus
 */

#ifndef _UAPI_LINUX_IPTS_H
#define _UAPI_LINUX_IPTS_H

#include <linux/ioctl.h>
#include <linux/types.h>

struct ipts_device_info {
	__u16 vendor;
	__u16 product;
	__u32 version;
	__u32 buffer_size;
	__u8 max_contacts;

	/* For future expansion */
	__u8 reserved[19];
};

#define IPTS_BUFFERS 16

#define IPTS_IOCTL_GET_DEVICE_READY _IOR(0x86, 0x01, __u8)
#define IPTS_IOCTL_GET_DEVICE_INFO  _IOR(0x86, 0x02, struct ipts_device_info)
#define IPTS_IOCTL_GET_DOORBELL	    _IOR(0x86, 0x03, __u32)
#define IPTS_IOCTL_SEND_FEEDBACK    _IO(0x86, 0x04)
#define IPTS_IOCTL_SEND_RESET	    _IO(0x86, 0x05)

#endif /* _UAPI_LINUX_IPTS_H */
