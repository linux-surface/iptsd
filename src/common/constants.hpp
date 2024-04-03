// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_CONSTANTS_HPP
#define IPTSD_COMMON_CONSTANTS_HPP

#include <type_traits>

template <class T>
constexpr T Zero()
{
	if constexpr (std::is_scalar_v<T>)
		return static_cast<T>(0);
	else
		return T::Zero();
}

template <class T>
constexpr T One()
{
	if constexpr (std::is_scalar_v<T>)
		return static_cast<T>(1);
	else
		return T::Ones();
}

#endif // IPTSD_COMMON_CONSTANTS_HPP
