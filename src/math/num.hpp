/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_MATH_NUM_HPP
#define IPTSD_MATH_NUM_HPP

#include <common/types.hpp>

namespace iptsd::math {

template <typename> struct num {};

template <> struct num<f32> {
	static inline constexpr f32 zero = 0.0f;
	static inline constexpr f32 one = 1.0f;
	static inline constexpr f32 pi = 3.14159265359f;
	static inline constexpr f32 eps = 1e-20f;
};

template <> struct num<f64> {
	static inline constexpr f64 zero = 0.0;
	static inline constexpr f64 one = 1.0;
	static inline constexpr f64 pi = 3.14159265359;
	static inline constexpr f64 eps = 1e-40;
};

} /* namespace iptsd::math */

#endif /* IPTSD_MATH_NUM_HPP */
