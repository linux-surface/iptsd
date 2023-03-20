// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_CWRAP_HPP
#define IPTSD_COMMON_CWRAP_HPP

#include "types.hpp"

#include <cstddef>
#include <fcntl.h>
#include <gsl/gsl>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace iptsd::common {

inline int open(const std::string &file, const int args)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	return ::open(file.c_str(), args);
}

template <class T> inline ssize_t read(const int fd, const gsl::span<T> &dest)
{
	return ::read(fd, dest.data(), dest.size_bytes());
}

inline int ioctl(const int fd, const unsigned long rq)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	return ::ioctl(fd, rq, nullptr);
}

template <class T> inline int ioctl(const int fd, const unsigned long rq, T data)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	return ::ioctl(fd, rq, data);
}

} // namespace iptsd::common

#endif // IPTSD_COMMON_CWRAP_HPP
