// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_ERROR_HPP
#define IPTSD_COMMON_ERROR_HPP

#include <fmt/format.h>

#include <stdexcept>

namespace iptsd::common {

/*!
 * A generic exception class that uses error codes instead of strings.
 *
 * Using error codes allows a typed comparison of errors, as well as catching a specific error
 * code directly, instead of catching them all and having to compare the contained string.
 *
 * @tparam T The error code that was thrown.
 */
template <auto T>
class Error : public std::runtime_error {
public:
	template <class... Args>
	Error(Args... args) : std::runtime_error {msg(args...)}
	{
		/*
		 * GCC doesn't see this block as empty, so adding a semicolon to the end to force
		 * clang-format to put it onto a single line makes GCC fail.
		 */
	}

private:
	template <class... Args>
	static std::string msg(Args... args)
	{
		/*
		 * First run the error code through format, to get a string representation.
		 * This representation might itself be a format string and accept parameters.
		 *
		 * NOTE:
		 *
		 * This was supposed to be fmt::format("{}", T), but version 9 of the fmt
		 * library only uses the format_as method when it returns an integer type.
		 *
		 * Instead of wasting lines implementing the full blown fmt::formatter type,
		 * we just call the format_as method here directly, just like fmt would do.
		 *
		 * That said I have no idea how this call actually works.
		 * If you are reading this and wondering where format_as is defined, it is
		 * next to the enum class holding the error codes.
		 */
		std::string error = format_as(T);

		/*
		 * If we have any parameters, try to format the error string as if it were
		 * another format string. Otherwise return it as-is to avoid exceptions.
		 */
		if (sizeof...(args) == 0)
			return error;

		return fmt::format(fmt::runtime(error), args...);
	}
};

} // namespace iptsd::common

#endif // IPTSD_COMMON_ERROR_HPP
