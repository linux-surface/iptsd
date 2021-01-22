/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_UINPUT_DEVICE_HPP_
#define _IPTSD_UINPUT_DEVICE_HPP_

#include <cstdint>
#include <string>

class UinputDevice {
public:
	int fd;

	std::string name;
	uint16_t vendor;
	uint16_t product;
	uint32_t version;

	UinputDevice(void);
	~UinputDevice(void);

	void set_evbit(int32_t ev);
	void set_propbit(int32_t prop);
	void set_keybit(int32_t key);
	void set_absinfo(uint16_t code, int32_t min, int32_t max, int32_t res);
	void create(void);
	void emit(uint16_t type, uint16_t key, int32_t value);
};

#endif /* _IPTSD_UINPUT_DEVICE_HPP_ */
