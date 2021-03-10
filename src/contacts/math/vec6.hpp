#pragma once

#include "types.hpp"
#include "math/num.hpp"
#include <common/access.hpp>


namespace iptsd::contacts::math {

template<class T>
struct Vec6 {
public:
    std::array<T, 6> data;

public:
    using value_type = T;

public:
    constexpr auto operator[] (index_t i) -> T&;
    constexpr auto operator[] (index_t i) const -> T const&;
};


template<class T>
inline constexpr auto Vec6<T>::operator[] (index_t i) -> T&
{
    return common::access<T, index_t>(data, data.size(), i);
}

template<class T>
inline constexpr auto Vec6<T>::operator[] (index_t i) const -> T const&
{
    return common::access<T, index_t>(data, data.size(), i);
}


template<class T>
struct num<Vec6<T>> {
    static inline constexpr Vec6<T> zero = {
            num<T>::zero, num<T>::zero, num<T>::zero,
            num<T>::zero, num<T>::zero, num<T>::zero };
};

} /* namespace iptsd::contacts::math */


/* imports */
namespace iptsd::contacts {

using math::Vec6;

} /* namespace iptsd::contacts */
