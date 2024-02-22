// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_DEVICE_ERRORS_HPP
#define IPTSD_CORE_LINUX_DEVICE_ERRORS_HPP

#include <common/types.hpp>

#include <string>

namespace iptsd::core::linux::device {

enum class Error : u8 {
	EndOfData,
};

inline std::string format_as(Error err)
{
	switch (err) {
	case Error::EndOfData:
		return "core: linux: devices: No further data available!";
	default:
		return "core: linux: devices: Invalid error code!";
	}
}

}; // namespace iptsd::core::linux::device

#endif // IPTSD_CORE_LINUX_DEVICE_ERRORS_HPP
