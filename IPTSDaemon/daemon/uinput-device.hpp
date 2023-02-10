/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_UINPUT_DEVICE_HPP
#define IPTSD_DAEMON_UINPUT_DEVICE_HPP

#include <common/types.hpp>

#include <string>

namespace iptsd::daemon {

class UinputDevice {
public:
	int fd;

	std::string name;
	u16 vendor = 0;
	u16 product = 0;
	u32 version = 0;

public:
	UinputDevice();
	~UinputDevice();

	void set_evbit(i32 ev) const;
	void set_propbit(i32 prop) const;
	void set_keybit(i32 key) const;
	void set_absinfo(u16 code, i32 min, i32 max, i32 res) const;
	void create() const;
	void emit(u16 type, u16 key, i32 value) const;
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_UINPUT_DEVICE_HPP */
