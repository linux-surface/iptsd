/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_MATH_VEC6_HPP
#define IPTSD_MATH_VEC6_HPP

#include "num.hpp"

#include <common/types.hpp>

#include <array>

namespace iptsd::math {

template <class T> struct Vec6 {
public:
	std::array<T, 6> data;

public:
	using value_type = T;

public:
	constexpr auto operator[](index_t i) -> T &;
	constexpr auto operator[](index_t i) const -> T const &;
};

template <class T> inline constexpr auto Vec6<T>::operator[](index_t i) -> T &
{
	return this->data.at(i);
}

template <class T> inline constexpr auto Vec6<T>::operator[](index_t i) const -> T const &
{
	return this->data.at(i);
}

template <class T> struct num<Vec6<T>> {
	static inline constexpr Vec6<T> zero = {num<T>::zero, num<T>::zero, num<T>::zero,
						num<T>::zero, num<T>::zero, num<T>::zero};
};

} /* namespace iptsd::math */

#endif /* IPTSD_MATH_VEC6_HPP */
