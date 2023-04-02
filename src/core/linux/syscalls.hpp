// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_SYSCALLS_HPP
#define IPTSD_CORE_LINUX_SYSCALLS_HPP

#include <common/types.hpp>

#include <gsl/gsl>

#include <linux/input.h>
#include <sys/ioctl.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <string_view>
#include <system_error>
#include <unistd.h>

namespace iptsd::core::linux::syscalls {

namespace impl {

/*!
 * Returns an std::error_code corresponding to the value of errno.
 *
 * @return The error code object to be used with @ref std::system_error.
 */
inline std::error_code last_error()
{
	return std::error_code {errno, std::system_category()};
}

} // namespace impl

inline int open(const std::filesystem::path &file, const int args)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	const int ret = ::open(file.c_str(), args);
	if (ret == -1)
		throw std::system_error {impl::last_error()};

	return ret;
}

template <class T>
inline isize read(const int fd, gsl::span<T> dest)
{
	const isize ret = ::read(fd, dest.data(), dest.size_bytes());
	if (ret == -1)
		throw std::system_error {impl::last_error()};

	return ret;
}

template <class T>
inline isize read(const int fd, T &dest)
{
	return read(fd, gsl::span {&dest, 1});
}

template <class T>
inline isize write(const int fd, const gsl::span<T> data)
{
	const isize ret = ::write(fd, data.data(), data.size_bytes());
	if (ret == -1)
		throw std::system_error {impl::last_error()};

	return ret;
}

template <class T>
inline isize write(const int fd, const T &data)
{
	return write(fd, gsl::span {&data, 1});
}

inline int close(const int fd)
{
	const int ret = ::close(fd);
	if (ret == -1)
		throw std::system_error {impl::last_error()};

	return ret;
}

template <class T = std::nullptr_t>
inline int ioctl(const int fd, const unsigned long rq, T data = nullptr)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
	const int ret = ::ioctl(fd, rq, data);
	if (ret == -1)
		throw std::system_error {impl::last_error()};

	return ret;
}

inline int sigaction(const int sig, const struct sigaction *act, struct sigaction *oact = nullptr)
{
	const int ret = ::sigaction(sig, act, oact);
	if (ret == -1)
		throw std::system_error {impl::last_error()};

	return ret;
}

} // namespace iptsd::core::linux::syscalls

#endif // IPTSD_CORE_LINUX_SYSCALLS_HPP
