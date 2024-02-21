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
	Error(Args... args) : std::runtime_error {msg(args...)} {};

private:
	template <class... Args>
	static std::string msg(Args... args)
	{
		/*
		 * First run the error code through format, to get a string representation.
		 * This representation might itself be a format string and accept parameters.
		 */
		std::string error = fmt::format("{}", T);

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
