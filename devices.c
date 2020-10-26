// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "devices.h"
#include "stylus-processing.h"
#include "syscall.h"
#include "touch-processing.h"

static int iptsd_devices_res(int virt, int phys)
{
	double res = (double)(virt * 10) / (double)phys;
	return (int)roundf(res);
}

static int iptsd_devices_create_stylus(struct iptsd_stylus_device *stylus,
		struct iptsd_device_config config)
{
	struct uinput_setup setup;
	struct uinput_abs_setup abs_setup;

	int file = iptsd_syscall_open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (file < 0)
		return file;

	stylus->dev = file;
	memset(&setup, 0, sizeof(struct uinput_setup));
	memset(&abs_setup, 0, sizeof(struct uinput_abs_setup));

	iptsd_syscall_ioctl(file, UI_SET_EVBIT, (void *)EV_KEY);
	iptsd_syscall_ioctl(file, UI_SET_EVBIT, (void *)EV_ABS);

	iptsd_syscall_ioctl(file, UI_SET_PROPBIT, (void *)INPUT_PROP_DIRECT);
	iptsd_syscall_ioctl(file, UI_SET_PROPBIT, (void *)INPUT_PROP_POINTER);

	iptsd_syscall_ioctl(file, UI_SET_KEYBIT, (void *)BTN_TOUCH);
	iptsd_syscall_ioctl(file, UI_SET_KEYBIT, (void *)BTN_STYLUS);
	iptsd_syscall_ioctl(file, UI_SET_KEYBIT, (void *)BTN_TOOL_PEN);
	iptsd_syscall_ioctl(file, UI_SET_KEYBIT, (void *)BTN_TOOL_RUBBER);

	abs_setup.code = ABS_X;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 9600;
	abs_setup.absinfo.resolution = iptsd_devices_res(9600, config.width);
	iptsd_syscall_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_Y;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 7200;
	abs_setup.absinfo.resolution = iptsd_devices_res(7200, config.height);
	iptsd_syscall_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_PRESSURE;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 4096;
	abs_setup.absinfo.resolution = 0;
	iptsd_syscall_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_TILT_X;
	abs_setup.absinfo.minimum = -9000;
	abs_setup.absinfo.maximum = 9000;
	abs_setup.absinfo.resolution = 5730;
	iptsd_syscall_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_TILT_Y;
	abs_setup.absinfo.minimum = -9000;
	abs_setup.absinfo.maximum = 9000;
	abs_setup.absinfo.resolution = 5730;
	iptsd_syscall_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MISC;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 65535;
	abs_setup.absinfo.resolution = 0;
	iptsd_syscall_ioctl(file, UI_ABS_SETUP, &abs_setup);

	setup.id.bustype = BUS_VIRTUAL;
	setup.id.vendor = config.vendor;
	setup.id.product = config.product;
	setup.id.version = config.version;
	strcpy(setup.name, "IPTS Stylus");

	int ret = iptsd_syscall_ioctl(file, UI_DEV_SETUP, &setup);
	if (ret < 0)
		return ret;

	return iptsd_syscall_ioctl(file, UI_DEV_CREATE, NULL);
}

static int iptsd_devices_create_touch(struct iptsd_touch_device *touch,
		struct iptsd_device_config config)
{
	struct uinput_setup setup;
	struct uinput_abs_setup abs_setup;

	int file = iptsd_syscall_open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (file < 0)
		return file;

	touch->dev = file;
	touch->processor.max_contacts = config.max_contacts;
	touch->processor.invert_x = config.invert_x;
	touch->processor.invert_y = config.invert_y;

	int ret = iptsd_touch_processing_init(&touch->processor);
	if (ret < 0)
		return ret;

	memset(&setup, 0, sizeof(struct uinput_setup));
	memset(&abs_setup, 0, sizeof(struct uinput_abs_setup));

	iptsd_syscall_ioctl(file, UI_SET_EVBIT, (void *)EV_ABS);
	iptsd_syscall_ioctl(file, UI_SET_PROPBIT, (void *)INPUT_PROP_DIRECT);

	abs_setup.code = ABS_MT_SLOT;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = config.max_contacts;
	abs_setup.absinfo.resolution = 0;
	iptsd_syscall_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_TRACKING_ID;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = config.max_contacts;
	abs_setup.absinfo.resolution = 0;
	iptsd_syscall_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_POSITION_X;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 9600;
	abs_setup.absinfo.resolution = iptsd_devices_res(9600, config.width);
	iptsd_syscall_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_POSITION_Y;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 7200;
	abs_setup.absinfo.resolution = iptsd_devices_res(7200, config.height);
	iptsd_syscall_ioctl(file, UI_ABS_SETUP, &abs_setup);

	setup.id.bustype = BUS_VIRTUAL;
	setup.id.vendor = config.vendor;
	setup.id.product = config.product;
	setup.id.version = config.version;
	strcpy(setup.name, "IPTS Touch");

	ret = iptsd_syscall_ioctl(file, UI_DEV_SETUP, &setup);
	if (ret < 0)
		return ret;

	return iptsd_syscall_ioctl(file, UI_DEV_CREATE, NULL);
}

int iptsd_devices_add_stylus(struct iptsd_devices *devices, uint32_t serial)
{
	struct iptsd_stylus_device *stylus = NULL;

	for (int i = 0; i < IPTSD_MAX_STYLI; i++) {
		if (devices->styli[i].active)
			continue;

		stylus = &devices->styli[i];
	}

	if (!stylus)
		return 0;

	stylus->active = true;
	stylus->serial = serial;
	devices->active_stylus = stylus;

	iptsd_stylus_processing_flush(&stylus->processor);

	return iptsd_devices_create_stylus(stylus, devices->config);
}

int iptsd_devices_emit(int fd, int type, int code, int val)
{
	struct input_event ie;

	memset(&ie, 0, sizeof(struct input_event));

	ie.type = type;
	ie.code = code;
	ie.value = val;

	return iptsd_syscall_write(fd, &ie, sizeof(struct input_event));
}

int iptsd_devices_create(struct iptsd_devices *devices)
{
	if (devices->config.width == 0 || devices->config.height == 0) {
		fprintf(stderr, "Display size is 0!\n");
		return -EINVAL;
	}

	int ret = iptsd_devices_create_touch(&devices->touch, devices->config);
	if (ret < 0) {
		fprintf(stderr, "Failed to create touch device: %s\n",
				iptsd_syscall_strerr(ret));
		return ret;
	}

	ret = iptsd_devices_add_stylus(devices, 0);
	if (ret < 0) {
		fprintf(stderr, "Failed to create stylus: %s\n",
				iptsd_syscall_strerr(ret));
		return ret;
	}

	return 0;
}

void iptsd_devices_destroy(struct iptsd_devices *devices)
{
	iptsd_syscall_ioctl(devices->touch.dev, UI_DEV_DESTROY, NULL);
	iptsd_syscall_close(devices->touch.dev);

	iptsd_touch_processing_free(&devices->touch.processor);

	for (int i = 0; i < IPTSD_MAX_STYLI; i++) {
		struct iptsd_stylus_device stylus = devices->styli[i];

		if (!stylus.active)
			continue;

		iptsd_syscall_ioctl(stylus.dev, UI_DEV_DESTROY, NULL);
		iptsd_syscall_close(stylus.dev);
	}
}

