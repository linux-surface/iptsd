/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_COMMON_UTILS_ACCESS_HPP_
#define _IPTSD_COMMON_UTILS_ACCESS_HPP_

#include <common/compiler.hpp>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <array>
#include <stdexcept>


namespace iptsd::utils::access {

enum class AccessMode {
	Checked,
	Unchecked,
};

#ifdef IPTSD_CONFIG_ACCESS_CHECKS
inline static constexpr AccessMode mode = AccessMode::Checked;
#else
inline static constexpr AccessMode mode = AccessMode::Unchecked;
#endif


namespace impl {

/*
 * Note: These functions have been extracted specifically to ensure that
 * performance with access checks enabled does not suffer too much. We
 * generally expect access to be valid, so tell the compiler to never inline
 * these functions and put them on a cold path.
 *
 * This improves performance significantly for tightly repeated container
 * accesses.
 */

template<class I>
[[noreturn, IPTSD_COLD, IPTSD_NOINLINE]]
inline auto blow_up(I size, I i) {
	auto buf = std::array<char, 128> {};

	fmt::format_to_n(buf.data(), buf.size(), "invalid access: size is {}, index is {}", size, i);

	throw std::out_of_range { buf.data() };
}

} /* namespace impl */


template<class I>
[[IPTSD_ALWAYS_INLINE]]
inline void ensure(I size, I i)
{
	if constexpr (mode == AccessMode::Unchecked) {
		return;
	}

	if (I { 0 } <= i && i < size) [[IPTSD_LIKELY]] {
		return;
	}

	impl::blow_up(size, i);
}

template<class V, class I, class T>
[[IPTSD_ALWAYS_INLINE]]
inline constexpr auto access(T const& data, I size, I i) -> V const&
{
	ensure(size, i);

	return data[i];
}

template<class V, class I, class T>
[[IPTSD_ALWAYS_INLINE]]
inline constexpr auto access(T& data, I size, I i) -> V&
{
	ensure(size, i);

	return data[i];
}

} /* namespace iptsd::utils::access */

#endif /* _IPTSD_COMMON_UTILS_ACCESS_HPP_ */
