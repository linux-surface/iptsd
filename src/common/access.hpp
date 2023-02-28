/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_COMMON_ACCESS_HPP
#define IPTSD_COMMON_ACCESS_HPP

namespace iptsd::common {

template <class V, class I, class T>
[[gnu::always_inline]] inline constexpr V &unchecked(T &data, I index)
{
#ifdef IPTSD_CONFIG_FORCE_ACCESS_CHECKS
	return data[index];
#else
	return data.data()[index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
#endif
}

template <class V, class I, class T>
[[gnu::always_inline]] inline constexpr const V &unchecked(const T &data, I index)
{
#ifdef IPTSD_CONFIG_FORCE_ACCESS_CHECKS
	return data[index];
#else
	return data.data()[index]; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
#endif
}

} /* namespace iptsd::common */

#endif /* IPTSD_COMMON_ACCESS_HPP */
