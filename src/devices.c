// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "devices.h"
#include "utils.h"
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

	int file = iptsd_utils_open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (file < 0) {
		iptsd_err(file, "Failed to open uinput device");
		return file;
	}

	stylus->dev = file;
	memset(&setup, 0, sizeof(struct uinput_setup));
	memset(&abs_setup, 0, sizeof(struct uinput_abs_setup));

	iptsd_utils_ioctl(file, UI_SET_EVBIT, (void *)EV_KEY);
	iptsd_utils_ioctl(file, UI_SET_EVBIT, (void *)EV_ABS);

	iptsd_utils_ioctl(file, UI_SET_PROPBIT, (void *)INPUT_PROP_DIRECT);
	iptsd_utils_ioctl(file, UI_SET_PROPBIT, (void *)INPUT_PROP_POINTER);

	iptsd_utils_ioctl(file, UI_SET_KEYBIT, (void *)BTN_TOUCH);
	iptsd_utils_ioctl(file, UI_SET_KEYBIT, (void *)BTN_STYLUS);
	iptsd_utils_ioctl(file, UI_SET_KEYBIT, (void *)BTN_TOOL_PEN);
	iptsd_utils_ioctl(file, UI_SET_KEYBIT, (void *)BTN_TOOL_RUBBER);

	abs_setup.code = ABS_X;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 9600;
	abs_setup.absinfo.resolution = iptsd_devices_res(9600, config.width);
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_Y;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 7200;
	abs_setup.absinfo.resolution = iptsd_devices_res(7200, config.height);
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_PRESSURE;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 4096;
	abs_setup.absinfo.resolution = 0;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_TILT_X;
	abs_setup.absinfo.minimum = -9000;
	abs_setup.absinfo.maximum = 9000;
	abs_setup.absinfo.resolution = 5730;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_TILT_Y;
	abs_setup.absinfo.minimum = -9000;
	abs_setup.absinfo.maximum = 9000;
	abs_setup.absinfo.resolution = 5730;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MISC;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 65535;
	abs_setup.absinfo.resolution = 0;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	setup.id.bustype = BUS_VIRTUAL;
	setup.id.vendor = config.vendor;
	setup.id.product = config.product;
	setup.id.version = config.version;
	strcpy(setup.name, "IPTS Stylus");

	int ret = iptsd_utils_ioctl(file, UI_DEV_SETUP, &setup);
	if (ret < 0) {
		iptsd_err(ret, "UI_DEV_SETUP failed");
		return ret;
	}

	ret = iptsd_utils_ioctl(file, UI_DEV_CREATE, NULL);
	if (ret < 0)
		iptsd_err(ret, "UI_DEV_CREATE failed");

	return ret;
}

static int iptsd_devices_create_touch(struct iptsd_touch_device *touch,
		struct iptsd_device_config config)
{
	struct uinput_setup setup;
	struct uinput_abs_setup abs_setup;

	int file = iptsd_utils_open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (file < 0) {
		iptsd_err(file, "Failed to open uinput device");
		return file;
	}

	touch->dev = file;
	touch->processor.max_contacts = config.max_contacts;
	touch->processor.invert_x = config.invert_x;
	touch->processor.invert_y = config.invert_y;

	int ret = iptsd_touch_processing_init(&touch->processor);
	if (ret < 0) {
		iptsd_err(ret, "Failed to init touch processing");
		return ret;
	}

	memset(&setup, 0, sizeof(struct uinput_setup));
	memset(&abs_setup, 0, sizeof(struct uinput_abs_setup));

	iptsd_utils_ioctl(file, UI_SET_EVBIT, (void *)EV_ABS);
	iptsd_utils_ioctl(file, UI_SET_PROPBIT, (void *)INPUT_PROP_DIRECT);

	abs_setup.code = ABS_MT_SLOT;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = config.max_contacts;
	abs_setup.absinfo.resolution = 0;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_TRACKING_ID;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = config.max_contacts;
	abs_setup.absinfo.resolution = 0;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_TOOL_TYPE;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = MT_TOOL_MAX;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_POSITION_X;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 9600;
	abs_setup.absinfo.resolution = iptsd_devices_res(9600, config.width);
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_POSITION_Y;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 7200;
	abs_setup.absinfo.resolution = iptsd_devices_res(7200, config.height);
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	setup.id.bustype = BUS_VIRTUAL;
	setup.id.vendor = config.vendor;
	setup.id.product = config.product;
	setup.id.version = config.version;
	strcpy(setup.name, "IPTS Touch");

	ret = iptsd_utils_ioctl(file, UI_DEV_SETUP, &setup);
	if (ret < 0) {
		iptsd_err(ret, "UI_DEV_SETUP failed");
		return ret;
	}

	ret = iptsd_utils_ioctl(file, UI_DEV_CREATE, NULL);
	if (ret < 0)
		iptsd_err(ret, "UI_DEV_CREATE failed");

	return ret;
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

	int ret = iptsd_devices_create_stylus(stylus, devices->config);
	if (ret < 0)
		iptsd_err(ret, "Failed to create stylus");
	else {
		int cone_id = devices->touch.processor.n_cones++;
		devices->touch.processor.rejection_cones[cone_id].pen_serial = serial;
	}

	return ret;
}

int iptsd_devices_emit(int fd, int type, int code, int val)
{
	struct input_event ie;

	memset(&ie, 0, sizeof(struct input_event));

	ie.type = type;
	ie.code = code;
	ie.value = val;

	int ret = iptsd_utils_write(fd, &ie, sizeof(struct input_event));
	if (ret < 0)
		iptsd_err(ret, "Failed to write input event");

	return ret;
}

int iptsd_devices_create(struct iptsd_devices *devices)
{
	if (devices->config.width == 0 || devices->config.height == 0) {
		iptsd_err(-EINVAL, "Display size is 0!\n");
		return -EINVAL;
	}

	int ret = iptsd_devices_create_touch(&devices->touch, devices->config);
	if (ret < 0) {
		iptsd_err(ret, "Failed to create touch device");
		return ret;
	}

	ret = iptsd_devices_add_stylus(devices, 0);
	if (ret < 0) {
		iptsd_err(ret, "Failed to create stylus");
		return ret;
	}

	return 0;
}

void iptsd_devices_destroy(struct iptsd_devices *devices)
{
	int ret = iptsd_utils_ioctl(devices->touch.dev, UI_DEV_DESTROY, NULL);
	if (ret < 0)
		iptsd_err(ret, "UI_DEV_DESTROY failed");

	ret = iptsd_utils_close(devices->touch.dev);
	if (ret < 0)
		iptsd_err(ret, "Closing uinput device failed");

	iptsd_touch_processing_free(&devices->touch.processor);

	for (int i = 0; i < IPTSD_MAX_STYLI; i++) {
		struct iptsd_stylus_device stylus = devices->styli[i];

		if (!stylus.active)
			continue;

		ret = iptsd_utils_ioctl(stylus.dev, UI_DEV_DESTROY, NULL);
		if (ret < 0)
			iptsd_err(ret, "UI_DEV_DESTROY failed");

		ret = iptsd_utils_close(stylus.dev);
		if (ret < 0)
			iptsd_err(ret, "Closing uinput device failed");
	}
}

