#pragma once

#include "types.hpp"

#include <array>
#include <cstdio>
#include <stdexcept>


namespace utils::access {

enum class access_mode {
    checked,
    unchecked,
};


#ifdef IPTSD_CONFIG_ACCESS_CHECKS
inline static constexpr access_mode mode = access_mode::checked;
#else
inline static constexpr access_mode mode = access_mode::unchecked;
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

[[noreturn, gnu::cold, gnu::noinline]]
inline auto blow_up(index_t size, index_t i) {
    auto buf = std::array<char, 128> {};

    std::snprintf(buf.data(), buf.size(), "invalid access: size is %d, index is %d", size, i);

    throw std::out_of_range { buf.data() };
}

[[noreturn, gnu::cold, gnu::noinline]]
inline auto blow_up(index2_t shape, index2_t i) {
    auto buf = std::array<char, 128> {};

    std::snprintf(buf.data(), buf.size(), "invalid access: size is [%d, %d], index is [%d, %d]",
                  shape.x, shape.y, i.x, i.y);

    throw std::out_of_range { buf.data() };
}

} /* namespace impl */


[[gnu::always_inline]]
inline void ensure(index_t size, index_t i)
{
    if constexpr (mode == access_mode::unchecked) {
        return;
    }

    if (0 <= i && i < size) [[gnu::likely]] {
        return;
    }

    impl::blow_up(size, i);
}

[[gnu::always_inline]]
inline void ensure(index2_t shape, index2_t i)
{
    if constexpr (mode == access_mode::unchecked) {
        return;
    }

    if (0 <= i.x && i.x < shape.x && 0 <= i.y && i.y < shape.y) [[gnu::likely]] {
        return;
    }

    impl::blow_up(shape, i);
}


template<class V, class T>
[[gnu::always_inline]]
inline constexpr auto access(T const& data, index_t size, index_t i) -> V const&
{
    ensure(size, i);

    return data[i];
}

template<class V, class T>
[[gnu::always_inline]]
inline constexpr auto access(T& data, index_t size, index_t i) -> V&
{
    ensure(size, i);

    return data[i];
}

template<class V, class T, class F>
[[gnu::always_inline]]
inline constexpr auto access(T const& data, F ravel, index2_t shape, index2_t i) -> V const&
{
    ensure(shape, i);

    return data[ravel(shape, i)];
}

template<class V, class T, class F>
[[gnu::always_inline]]
inline constexpr auto access(T& data, F ravel, index2_t shape, index2_t i) -> V&
{
    ensure(shape, i);

    return data[ravel(shape, i)];
}

} /* namespace utils::access */
