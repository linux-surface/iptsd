// SPDX-License-Identifier: GPL-2.0-or-later

#include "uinput-device.hpp"

#include "utils.hpp"

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <unistd.h>

UinputDevice::UinputDevice(void)
{
	int ret = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (ret == -1)
		throw Utils::cerror("Failed to open uinput device");

	this->fd = ret;
}

UinputDevice::~UinputDevice(void)
{
	ioctl(this->fd, UI_DEV_DESTROY, nullptr);
	close(this->fd);
}

void UinputDevice::set_evbit(int32_t ev)
{
	int ret = ioctl(this->fd, UI_SET_EVBIT, (int)ev);
	if (ret == -1)
		throw Utils::cerror("UI_SET_EVBIT failed");
}

void UinputDevice::set_keybit(int32_t key)
{
	int ret = ioctl(this->fd, UI_SET_KEYBIT, (int)key);
	if (ret == -1)
		throw Utils::cerror("UI_SET_KEYBIT failed");
}

void UinputDevice::set_propbit(int32_t prop)
{
	int ret = ioctl(this->fd, UI_SET_PROPBIT, (int)prop);
	if (ret == -1)
		throw Utils::cerror("UI_SET_PROPBIT failed");
}

void UinputDevice::set_absinfo(uint16_t code, int32_t min, int32_t max, int32_t res)
{
	struct uinput_abs_setup abs;

	memset(&abs, 0, sizeof(abs));

	abs.code = code;
	abs.absinfo.minimum = min;
	abs.absinfo.maximum = max;
	abs.absinfo.resolution = res;

	int ret = ioctl(this->fd, UI_ABS_SETUP, &abs);
	if (ret == -1)
		throw Utils::cerror("UI_ABS_SETUP failed");
}

void UinputDevice::create(void)
{
	struct uinput_setup setup;

	memset(&setup, 0, sizeof(setup));

	setup.id.bustype = BUS_VIRTUAL;
	setup.id.vendor = this->vendor;
	setup.id.product = this->product;
	setup.id.version = this->version;
	strncpy(setup.name, this->name.c_str(), this->name.length());

	int ret = ioctl(this->fd, UI_DEV_SETUP, &setup);
	if (ret == -1)
		throw Utils::cerror("UI_DEV_SETUP failed");

	ret = ioctl(this->fd, UI_DEV_CREATE, nullptr);
	if (ret == -1)
		throw Utils::cerror("UI_DEV_CREATE failed");
}

void UinputDevice::emit(uint16_t type, uint16_t key, int32_t value)
{
	struct input_event ie;

	memset(&ie, 0, sizeof(ie));

	ie.type = type;
	ie.code = key;
	ie.value = value;

	int ret = write(this->fd, &ie, sizeof(ie));
	if (ret == -1)
		throw Utils::cerror("Failed to write input event");
}
