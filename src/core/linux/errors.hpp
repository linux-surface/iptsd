// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_ERRORS_HPP
#define IPTSD_CORE_LINUX_ERRORS_HPP

#include <string>

namespace iptsd::core::linux {

enum class Error {
	ParsingFailed,
	ParsingTypeNotImplemented,
	RunnerInitError,
};

inline std::string format_as(Error err)
{
	switch (err) {
	case Error::ParsingFailed:
		return "core: linux: Failed to parse INI file {}!";
	case Error::ParsingTypeNotImplemented:
		return "core: linux: Parsing not implemented for type {}!";
	case Error::RunnerInitError:
		return "core: linux: Runner initialization failed!";
	default:
		return "core: linux: Invalid error code!";
	}
}

} // namespace iptsd::core::linux

#endif // IPTSD_CORE_LINUX_ERRORS_HPP
