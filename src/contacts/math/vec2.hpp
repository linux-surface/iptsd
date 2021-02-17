#pragma once

#include "math/num.hpp"

#include <cmath>
#include <iostream>


namespace math {

template<class T>
struct vec2_t {
public:
    T x, y;

public:
    using value_type = T;

public:
    inline constexpr auto operator= (vec2_t<T> const& rhs) -> vec2_t<T>& = default;

    constexpr auto operator+= (vec2_t<T> const& v) -> vec2_t<T>&;
    constexpr auto operator+= (T const& s) -> vec2_t<T>&;

    constexpr auto operator-= (vec2_t<T> const& v) -> vec2_t<T>&;
    constexpr auto operator-= (T const& s) -> vec2_t<T>&;

    constexpr auto operator*= (T const& s) -> vec2_t<T>&;
    constexpr auto operator/= (T const& s) -> vec2_t<T>&;

    constexpr auto dot(vec2_t<T> const& v) const -> T;
    constexpr auto norm_l2() const -> T;

    template<class S>
    constexpr auto cast() const -> vec2_t<S>;
};


template<class T>
inline constexpr auto vec2_t<T>::operator+= (vec2_t<T> const& v) -> vec2_t<T>&
{
    this->x += v.x;
    this->y += v.y;
    return *this;
}

template<class T>
inline constexpr auto vec2_t<T>::operator+= (T const& s) -> vec2_t<T>&
{
    this->x += s;
    this->y += s;
    return *this;
}

template<class T>
inline constexpr auto vec2_t<T>::operator-= (vec2_t<T> const& v) -> vec2_t<T>&
{
    this->x -= v.x;
    this->y -= v.y;
    return *this;
}

template<class T>
inline constexpr auto vec2_t<T>::operator-= (T const& s) -> vec2_t<T>&
{
    this->x -= s;
    this->y -= s;
    return *this;
}

template<class T>
inline constexpr auto vec2_t<T>::operator*= (T const& s) -> vec2_t<T>&
{
    this->x *= s;
    this->y *= s;
    return *this;
}

template<class T>
inline constexpr auto vec2_t<T>::operator/= (T const& s) -> vec2_t<T>&
{
    this->x /= s;
    this->y /= s;
    return *this;
}


template<class T>
inline constexpr auto vec2_t<T>::dot(vec2_t<T> const& v) const -> T
{
    return this->x * v.x + this->y * v.y;
}

template<class T>
inline constexpr auto vec2_t<T>::norm_l2() const -> T
{
    using std::sqrt;

    return sqrt(this->x * this->x + this->y * this->y);
}

template<class T>
template<class S>
inline constexpr auto vec2_t<T>::cast() const -> vec2_t<S>
{
    return { static_cast<S>(this->x), static_cast<S>(this->y) };
}


template<class T>
inline auto operator<< (std::ostream& os, vec2_t<T> const& v) -> std::ostream&
{
    return os << "[" << v.x << ", " << v.y << "]";
}

template<class T>
inline constexpr auto operator== (vec2_t<T> const& a, vec2_t<T> const& b) -> bool
{
    return a.x == b.x && a.y == b.y;
}

template<class T>
inline constexpr auto operator!= (vec2_t<T> const& a, vec2_t<T> const& b) -> bool
{
    return !(a == b);
}

template<class T>
inline constexpr auto operator+ (vec2_t<T> const& a, vec2_t<T> const& b) -> vec2_t<T>
{
    return { a.x + b.x, a.y + b.y };
}

template<class T>
inline constexpr auto operator+ (vec2_t<T> const& v, T const& s) -> vec2_t<T>
{
    return { v.x + s, v.y + s };
}

template<class T>
inline constexpr auto operator+ (T const& s, vec2_t<T> const& v) -> vec2_t<T>
{
    return { s + v.x, s + v.y };
}

template<class T>
inline constexpr auto operator- (vec2_t<T> const& a, vec2_t<T> const& b) -> vec2_t<T>
{
    return { a.x - b.x, a.y - b.y };
}

template<class T>
inline constexpr auto operator- (vec2_t<T> const& v, T const& s) -> vec2_t<T>
{
    return { v.x - s, v.y - s };
}

template<class T>
inline constexpr auto operator- (T const& s, vec2_t<T> const& v) -> vec2_t<T>
{
    return { s - v.x, s - v.y };
}

template<class T>
inline constexpr auto operator* (vec2_t<T> const& v, T const& s) -> vec2_t<T>
{
    return { v.x * s, v.y * s};
}

template<class T>
inline constexpr auto operator* (T const& s, vec2_t<T> const& v) -> vec2_t<T>
{
    return { s * v.x, s * v.y };
}

template<class T>
inline constexpr auto operator/ (vec2_t<T> const& v, T const& s) -> vec2_t<T>
{
    return { v.x / s, v.y / s};
}

template<class T>
inline constexpr auto operator/ (T const& s, vec2_t<T> const& v) -> vec2_t<T>
{
    return { s / v.x, s / v.y };
}


template<class T>
struct num<vec2_t<T>> {
    static inline constexpr vec2_t<T> zero = { num<T>::zero, num<T>::zero };
};

} /* namespace math */
