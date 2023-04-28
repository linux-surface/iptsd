// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_CASTS_HPP
#define IPTSD_COMMON_CASTS_HPP

#ifndef EIGEN_DEFAULT_TO_ROW_MAJOR
#define EIGEN_DEFAULT_TO_ROW_MAJOR
#endif

#include <Eigen/Eigen>
#include <gsl/gsl>

#include <type_traits>

namespace iptsd::casts {

template <class T>
constexpr inline std::make_signed_t<T> to_signed(const T value)
{
	using S = std::make_signed_t<T>;

	if constexpr (std::is_same_v<T, S>)
		return value;
	else
		return gsl::narrow<S>(value);
}

template <class T>
constexpr inline std::make_unsigned_t<T> to_unsigned(const T value)
{
	using U = std::make_unsigned_t<T>;

	if constexpr (std::is_same_v<T, U>)
		return value;
	else
		return gsl::narrow<U>(value);
}

template <class T>
constexpr inline Eigen::Index to_eigen(const T value)
{
	if constexpr (std::is_signed_v<Eigen::Index>)
		return casts::to_signed(value);
	else
		return casts::to_unsigned(value);
}

} // namespace iptsd::casts

#endif // IPTSD_COMMON_CASTS_HPP
