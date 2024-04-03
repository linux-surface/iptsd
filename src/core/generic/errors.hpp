// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_GENERIC_ERRORS_HPP
#define IPTSD_CORE_GENERIC_ERRORS_HPP

#include <common/types.hpp>

#include <string>

namespace iptsd::core {

enum class Error : u8 {
	InvalidScreenSize,
	InvalidNeutralValueAlgorithm,
};

inline std::string format_as(Error err)
{
	switch (err) {
	case Error::InvalidScreenSize:
		return "core: The screen size is 0! Is your device supported?";
	case Error::InvalidNeutralValueAlgorithm:
		return "core: The selected neutral value algorithm is invalid!";
	default:
		return "core: Invalid error code!";
	}
}

} // namespace iptsd::core

#endif // IPTSD_CORE_GENERIC_ERRORS_HPP
