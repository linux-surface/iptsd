/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_UINPUT_DEVICE_HPP
#define IPTSD_DAEMON_UINPUT_DEVICE_HPP

#include <common/types.hpp>

#include <string>

class UinputDevice {
public:
	int fd;

	std::string name;
	u16 vendor = 0;
	u16 product = 0;
	u32 version = 0;

	UinputDevice();
	~UinputDevice();

	void set_evbit(i32 ev);
	void set_propbit(i32 prop);
	void set_keybit(i32 key);
	void set_absinfo(u16 code, i32 min, i32 max, i32 res);
	void create();
	void emit(u16 type, u16 key, i32 value);
};

#endif /* IPTSD_DAEMON_UINPUT_DEVICE_HPP */
