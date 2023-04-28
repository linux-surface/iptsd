// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_CASTS_HPP
#define IPTSD_COMMON_CASTS_HPP

#ifndef EIGEN_DEFAULT_TO_ROW_MAJOR
#define EIGEN_DEFAULT_TO_ROW_MAJOR
#endif

#include "types.hpp"

#include <Eigen/Eigen>
#include <gsl/gsl>

#include <type_traits>

namespace iptsd::casts {

template <class To, class From>
constexpr inline To to(const From value)
{
	static_assert(!std::is_same_v<From, f64> || !std::is_same_v<To, f32>,
		      "f64 to f32 cannot be narrowed safely. Please use gsl::narrow_cast!");

	using Common = std::common_type_t<To, From>;

	// Can To contain From?
	if constexpr (std::is_same_v<Common, To>)
		return static_cast<To>(value);
	else
		return gsl::narrow<To>(value);
}

template <class T>
constexpr inline std::make_signed_t<T> to_signed(const T value)
{
	return to<std::make_signed_t<T>>(value);
}

template <class T>
constexpr inline std::make_unsigned_t<T> to_unsigned(const T value)
{
	return to<std::make_unsigned_t<T>>(value);
}

template <class T>
constexpr inline Eigen::Index to_eigen(const T value)
{
	return to<Eigen::Index>(value);
}

} // namespace iptsd::casts

#endif // IPTSD_COMMON_CASTS_HPP
