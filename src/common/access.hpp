/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_COMMON_ACCESS_HPP
#define IPTSD_COMMON_ACCESS_HPP

#include "compiler.hpp"

#include <array>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <stdexcept>

namespace iptsd::common {

template <class V, class I, class T>
[[gnu::always_inline]] inline constexpr auto unchecked(T &data, I index) -> V &
{
	return data.data()[index];
}

template <class V, class I, class T>
[[gnu::always_inline]] inline constexpr auto unchecked(const T &data, I index) -> const V &
{
	return data.data()[index];
}

} /* namespace iptsd::common */

#endif /* IPTSD_COMMON_ACCESS_HPP */
