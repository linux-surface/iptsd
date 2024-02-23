// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_DEVICE_HPP
#define IPTSD_HID_DEVICE_HPP

#include "descriptor.hpp"
#include "parser.hpp"

#include <common/types.hpp>

#include <gsl/gsl>

namespace iptsd::hid {

class Device {
private:
	Descriptor m_desc {};
	bool m_parsed = false;

public:
	virtual ~Device() = default;

	virtual u16 vendor() = 0;
	virtual u16 product() = 0;
	virtual std::string_view name() = 0;
	virtual gsl::span<u8> raw_descriptor() = 0;

	virtual usize read(gsl::span<u8> buffer) = 0;
	virtual void get_feature(gsl::span<u8> report) = 0;
	virtual void set_feature(gsl::span<u8> report) = 0;

	const Descriptor &descriptor()
	{
		if (!m_parsed) {
			hid::parse(this->raw_descriptor(), m_desc);
			m_parsed = true;
		}

		return m_desc;
	}
};

} // namespace iptsd::hid

#endif // IPTSD_HID_DEVICE_HPP
