/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_COMMON_CERROR_HPP_
#define _IPTSD_COMMON_CERROR_HPP_

#include <cerrno>
#include <string>
#include <system_error>

namespace iptsd::common {

inline auto cerror(std::string msg) -> std::system_error
{
	return std::system_error{ std::error_code{ errno, std::system_category() }, msg };
}

} /* namespace iptsd::common */

#endif /* _IPTSD_COMMON_CERROR_HPP_ */
