/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_COMMON_ACCESS_HPP
#define IPTSD_COMMON_ACCESS_HPP

#include <array>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <stdexcept>

namespace iptsd::common {

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

template <class I> [[noreturn, gnu::cold, gnu::noinline]] inline auto blow_up(I i, I begin, I end)
{
	std::string msg = fmt::format("invalid access: {} not in range {} to {}", i, begin, end);

	throw std::out_of_range(msg);
}

} /* namespace impl */

template <class I> [[gnu::always_inline]] inline void ensure(I i, I begin, I end)
{
	if constexpr (mode == AccessMode::Unchecked)
		return;

	if (begin <= i && i < end) [[likely]]
		return;

	impl::blow_up(i, begin, end);
}

template <class I> [[gnu::always_inline]] inline void ensure(I i, I size)
{
	ensure(i, I {0}, size);
}

template <class V, class I, class T>
[[gnu::always_inline]] inline constexpr auto access(T const &data, I size, I i) -> V const &
{
	ensure(i, size);

	return data[i];
}

template <class V, class I, class T>
[[gnu::always_inline]] inline constexpr auto access(T &data, I size, I i) -> V &
{
	ensure(i, size);

	return data[i];
}

template <class V, class I, class T, class F>
[[gnu::always_inline]] inline constexpr auto access(T const &data, F ravel, I shape, I i)
	-> V const &
{
	ensure(i, shape);

	return data[ravel(shape, i)];
}

template <class V, class I, class T, class F>
[[gnu::always_inline]] inline constexpr auto access(T &data, F ravel, I shape, I i) -> V &
{
	ensure(i, shape);

	return data[ravel(shape, i)];
}

} /* namespace iptsd::common */

#endif /* IPTSD_COMMON_ACCESS_HPP */
