/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_COMMON_CWRAP_HPP_
#define _IPTSD_COMMON_CWRAP_HPP_

#include "types.hpp"

#include <cstddef>
#include <fcntl.h>
#include <span>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace iptsd::common {

inline int open(const std::string &file, int args)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	return ::open(file.c_str(), args);
}

template <class T> inline ssize_t read(int fd, std::span<T> dest)
{
	return ::read(fd, dest.data(), dest.size_bytes());
}

inline int ioctl(int fd, unsigned long rq)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	return ::ioctl(fd, rq, nullptr);
}

template <class T> inline int ioctl(int fd, unsigned long rq, T data)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	void *b = reinterpret_cast<void *>(data);

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	return ::ioctl(fd, rq, b);
}

} // namespace iptsd::common

#endif /* _IPTSD_COMMON_CWRAP_HPP_ */
