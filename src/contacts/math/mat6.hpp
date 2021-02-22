#pragma once

#include "math/num.hpp"
#include "utils/access.hpp"


namespace iptsd::math {

template<class T>
struct Mat6 {
public:
    std::array<T, 6 * 6> data;

public:
    using value_type = T;

public:
    constexpr static auto identity() -> Mat6<T>;

    constexpr auto operator[] (index2_t i) -> T&;
    constexpr auto operator[] (index2_t i) const -> T const&;
};


template<class T>
inline constexpr auto Mat6<T>::identity() -> Mat6<T>
{
    auto const _0 = num<T>::zero;
    auto const _1 = num<T>::one;

    return {
        _1, _0, _0, _0, _0, _0,
        _0, _1, _0, _0, _0, _0,
        _0, _0, _1, _0, _0, _0,
        _0, _0, _0, _1, _0, _0,
        _0, _0, _0, _0, _1, _0,
        _0, _0, _0, _0, _0, _1,
    };
}

template<class T>
inline constexpr auto Mat6<T>::operator[] (index2_t i) -> T&
{
    auto const ravel = [](index2_t shape, index2_t i) {
        return i.x * shape.y + i.y;
    };

    return utils::access::access<T>(data, ravel, { 6, 6 }, i);
}

template<class T>
inline constexpr auto Mat6<T>::operator[] (index2_t i) const -> T const&
{
    auto const ravel = [](index2_t shape, index2_t i) {
        return i.x * shape.y + i.y;
    };

    return utils::access::access<T>(data, ravel, { 6, 6 }, i);
}

} /* namespace iptsd::math */


/* imports */
namespace iptsd {

using math::Mat6;

} /* namespace iptsd */
