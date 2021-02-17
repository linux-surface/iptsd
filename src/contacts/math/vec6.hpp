#pragma once

#include "types.hpp"
#include "math/num.hpp"
#include "utils/access.hpp"


namespace math {

template<class T>
struct vec6_t {
public:
    std::array<T, 6> data;

public:
    using value_type = T;

public:
    constexpr auto operator[] (index_t i) -> T&;
    constexpr auto operator[] (index_t i) const -> T const&;
};


template<class T>
inline constexpr auto vec6_t<T>::operator[] (index_t i) -> T&
{
    return utils::access::access<T>(data, data.size(), i);
}

template<class T>
inline constexpr auto vec6_t<T>::operator[] (index_t i) const -> T const&
{
    return utils::access::access<T>(data, data.size(), i);
}


template<class T>
struct num<vec6_t<T>> {
    static inline constexpr vec6_t<T> zero = {
            num<T>::zero, num<T>::zero, num<T>::zero,
            num<T>::zero, num<T>::zero, num<T>::zero };
};

} /* namespace math */
