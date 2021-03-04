// SPDX-License-Identifier: GPL-2.0-or-later

#include "uinput-device.hpp"

#include <common/types.hpp>
#include <common/cerror.hpp>

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
		throw iptsd::common::cerror("Failed to open uinput device");

	this->fd = ret;
}

UinputDevice::~UinputDevice(void)
{
	ioctl(this->fd, UI_DEV_DESTROY, nullptr);
	close(this->fd);
}

void UinputDevice::set_evbit(i32 ev)
{
	int ret = ioctl(this->fd, UI_SET_EVBIT, (int)ev);
	if (ret == -1)
		throw iptsd::common::cerror("UI_SET_EVBIT failed");
}

void UinputDevice::set_keybit(i32 key)
{
	int ret = ioctl(this->fd, UI_SET_KEYBIT, (int)key);
	if (ret == -1)
		throw iptsd::common::cerror("UI_SET_KEYBIT failed");
}

void UinputDevice::set_propbit(i32 prop)
{
	int ret = ioctl(this->fd, UI_SET_PROPBIT, (int)prop);
	if (ret == -1)
		throw iptsd::common::cerror("UI_SET_PROPBIT failed");
}

void UinputDevice::set_absinfo(u16 code, i32 min, i32 max, i32 res)
{
	struct uinput_abs_setup abs;

	std::memset(&abs, 0, sizeof(abs));

	abs.code = code;
	abs.absinfo.minimum = min;
	abs.absinfo.maximum = max;
	abs.absinfo.resolution = res;

	int ret = ioctl(this->fd, UI_ABS_SETUP, &abs);
	if (ret == -1)
		throw iptsd::common::cerror("UI_ABS_SETUP failed");
}

void UinputDevice::create(void)
{
	struct uinput_setup setup;

	std::memset(&setup, 0, sizeof(setup));

	setup.id.bustype = BUS_VIRTUAL;
	setup.id.vendor = this->vendor;
	setup.id.product = this->product;
	setup.id.version = this->version;
	std::strncpy(setup.name, this->name.c_str(), this->name.length());

	int ret = ioctl(this->fd, UI_DEV_SETUP, &setup);
	if (ret == -1)
		throw iptsd::common::cerror("UI_DEV_SETUP failed");

	ret = ioctl(this->fd, UI_DEV_CREATE, nullptr);
	if (ret == -1)
		throw iptsd::common::cerror("UI_DEV_CREATE failed");
}

void UinputDevice::emit(u16 type, u16 key, i32 value)
{
	struct input_event ie;

	std::memset(&ie, 0, sizeof(ie));

	ie.type = type;
	ie.code = key;
	ie.value = value;

	int ret = write(this->fd, &ie, sizeof(ie));
	if (ret == -1)
		throw iptsd::common::cerror("Failed to write input event");
}
