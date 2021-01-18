// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "devices.h"
#include "protocol.h"
#include "touch-processing.h"
#include "utils.h"

static int iptsd_devices_res(int virt, int phys)
{
	double res = (double)(virt * 10) / (double)phys;
	return (int)roundf(res);
}

static int iptsd_devices_create_stylus(struct iptsd_devices *devices,
				       struct iptsd_stylus_device *stylus)
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

	int res_x = iptsd_devices_res(IPTS_MAX_X, devices->config.width);
	int res_y = iptsd_devices_res(IPTS_MAX_Y, devices->config.height);

	abs_setup.code = ABS_X;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = IPTS_MAX_X;
	abs_setup.absinfo.resolution = res_x;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_Y;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = IPTS_MAX_Y;
	abs_setup.absinfo.resolution = res_y;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_PRESSURE;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 4096;
	abs_setup.absinfo.resolution = 0;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_TILT_X;
	abs_setup.absinfo.minimum = -9000;
	abs_setup.absinfo.maximum = 9000;
	abs_setup.absinfo.resolution = (int32_t)(18000 / M_PI);
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_TILT_Y;
	abs_setup.absinfo.minimum = -9000;
	abs_setup.absinfo.maximum = 9000;
	abs_setup.absinfo.resolution = (int32_t)(18000 / M_PI);
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MISC;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 65535;
	abs_setup.absinfo.resolution = 0;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	setup.id.bustype = BUS_VIRTUAL;
	setup.id.vendor = devices->device_info.vendor;
	setup.id.product = devices->device_info.product;
	setup.id.version = devices->device_info.version;
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

static int iptsd_devices_create_touch(struct iptsd_devices *devices,
				      struct iptsd_touch_device *touch)
{
	struct uinput_setup setup;
	struct uinput_abs_setup abs_setup;

	int file = iptsd_utils_open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (file < 0) {
		iptsd_err(file, "Failed to open uinput device");
		return file;
	}

	touch->dev = file;
	touch->processor.config = devices->config;
	touch->processor.device_info = devices->device_info;

	int ret = iptsd_touch_processing_init(&touch->processor);
	if (ret < 0) {
		iptsd_err(ret, "Failed to init touch processing");
		return ret;
	}

	memset(&setup, 0, sizeof(struct uinput_setup));
	memset(&abs_setup, 0, sizeof(struct uinput_abs_setup));

	iptsd_utils_ioctl(file, UI_SET_EVBIT, (void *)EV_ABS);
	iptsd_utils_ioctl(file, UI_SET_EVBIT, (void *)EV_KEY);

	iptsd_utils_ioctl(file, UI_SET_KEYBIT, (void *)BTN_TOUCH);
	iptsd_utils_ioctl(file, UI_SET_PROPBIT, (void *)INPUT_PROP_DIRECT);

	float diag = sqrtf(devices->config.width * devices->config.width +
			   devices->config.height * devices->config.height);

	int res_x = iptsd_devices_res(IPTS_MAX_X, devices->config.width);
	int res_y = iptsd_devices_res(IPTS_MAX_Y, devices->config.height);
	int res_d = iptsd_devices_res(IPTS_DIAGONAL, diag);

	abs_setup.code = ABS_MT_SLOT;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = devices->device_info.max_contacts;
	abs_setup.absinfo.resolution = 0;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_TRACKING_ID;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = devices->device_info.max_contacts;
	abs_setup.absinfo.resolution = 0;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_POSITION_X;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = IPTS_MAX_X;
	abs_setup.absinfo.resolution = res_x;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_POSITION_Y;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = IPTS_MAX_Y;
	abs_setup.absinfo.resolution = res_y;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_TOOL_TYPE;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = MT_TOOL_MAX;
	abs_setup.absinfo.resolution = 0;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_TOOL_X;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = IPTS_MAX_X;
	abs_setup.absinfo.resolution = res_x;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_TOOL_Y;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = IPTS_MAX_Y;
	abs_setup.absinfo.resolution = res_y;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_ORIENTATION;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = 180;
	abs_setup.absinfo.resolution = 0;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_TOUCH_MAJOR;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = IPTS_DIAGONAL;
	abs_setup.absinfo.resolution = res_d;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_MT_TOUCH_MINOR;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = IPTS_DIAGONAL;
	abs_setup.absinfo.resolution = res_d;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_X;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = IPTS_MAX_X;
	abs_setup.absinfo.resolution = res_x;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	abs_setup.code = ABS_Y;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = IPTS_MAX_Y;
	abs_setup.absinfo.resolution = res_y;
	iptsd_utils_ioctl(file, UI_ABS_SETUP, &abs_setup);

	setup.id.bustype = BUS_VIRTUAL;
	setup.id.vendor = devices->device_info.vendor;
	setup.id.product = devices->device_info.product;
	setup.id.version = devices->device_info.version;
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
	int i;
	struct iptsd_stylus_device *stylus = NULL;

	for (i = 0; i < IPTSD_MAX_STYLI; i++) {
		if (devices->styli[i].active)
			continue;

		stylus = &devices->styli[i];
		break;
	}

	if (!stylus)
		return 0;

	stylus->active = true;
	stylus->serial = serial;
	stylus->cone = &devices->touch.processor.rejection_cones[i];
	devices->active_stylus = stylus;

	int ret = iptsd_devices_create_stylus(devices, stylus);
	if (ret < 0)
		iptsd_err(ret, "Failed to create stylus");

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

	int ret = iptsd_devices_create_touch(devices, &devices->touch);
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
