// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_ERRORS_HPP
#define IPTSD_CORE_LINUX_ERRORS_HPP

#include <common/types.hpp>

#include <string>

namespace iptsd::core::linux {

enum class Error : u8 {
	ParsingFailed,
	ParsingTypeNotImplemented,
	RunnerInitError,

	SyscallOpenFailed,
	SyscallReadFailed,
	SyscallWriteFailed,
	SyscallCloseFailed,
	SyscallIoctlFailed,
	SyscallSigactionFailed,
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
	case Error::SyscallOpenFailed:
		return "core: linux: Opening file {} failed: {}";
	case Error::SyscallReadFailed:
		return "core: linux: Reading from file failed: {}";
	case Error::SyscallWriteFailed:
		return "core: linux: Writing to file failed: {}";
	case Error::SyscallCloseFailed:
		return "core: linux: Closing file failed: {}";
	case Error::SyscallIoctlFailed:
		return "core: linux: IOCTL {} failed: {}";
	case Error::SyscallSigactionFailed:
		return "core: linux: Sigaction for signal {} failed: {}";
	default:
		return "core: linux: Invalid error code!";
	}
}

} // namespace iptsd::core::linux

#endif // IPTSD_CORE_LINUX_ERRORS_HPP
