#pragma once

#include "math/num.hpp"
#include "math/poly2.hpp"
#include "math/vec2.hpp"

#include <iostream>
#include <optional>


namespace math {

template<class T>
struct eigen2_t {
    std::array<T, 2>         w;
    std::array<vec2_t<T>, 2> v;
};


template<class T>
struct mat2s_t {
public:
    T xx, xy, yy;

public:
    using value_type = T;

public:
    static constexpr auto identity() -> mat2s_t<T>;

    constexpr auto operator+= (mat2s_t<T> const& m) -> mat2s_t<T>&;
    constexpr auto operator+= (T const& s) -> mat2s_t<T>&;

    constexpr auto operator-= (mat2s_t<T> const& m) -> mat2s_t<T>&;
    constexpr auto operator-= (T const& s) -> mat2s_t<T>&;

    constexpr auto operator*= (T const& s) -> mat2s_t<T>&;
    constexpr auto operator/= (T const& s) -> mat2s_t<T>&;

    constexpr auto vtmv(vec2_t<T> const& v) const -> T;

    constexpr auto inverse(T eps=num<T>::eps) const -> std::optional<mat2s_t<T>>;

    constexpr auto det() const -> T;
    constexpr auto trace() const -> T;

    constexpr auto eigen(T eps=num<T>::eps) const -> eigen2_t<T>;
    constexpr auto eigenvalues(T eps=num<T>::eps) const -> std::array<T, 2>;
    constexpr auto eigenvector(T eigenvalue) const -> vec2_t<T>;

    template<class S>
    constexpr auto cast() const -> mat2s_t<S>;
};


template<class T>
inline constexpr auto mat2s_t<T>::identity() -> mat2s_t<T>
{
    return { num<T>::one, num<T>::zero, num<T>::one };
}


template<class T>
inline constexpr auto mat2s_t<T>::operator+= (mat2s_t<T> const& m) -> mat2s_t<T>&
{
    this->xx += m.xx;
    this->xy += m.xy;
    this->yy += m.yy;
    return *this;
}

template<class T>
inline constexpr auto mat2s_t<T>::operator+= (T const& s) -> mat2s_t<T>&
{
    this->xx += s;
    this->xy += s;
    this->yy += s;
    return *this;
}

template<class T>
inline constexpr auto mat2s_t<T>::operator-= (mat2s_t<T> const& m) -> mat2s_t<T>&
{
    this->xx -= m.xx;
    this->xy -= m.xy;
    this->yy -= m.yy;
    return *this;
}

template<class T>
inline constexpr auto mat2s_t<T>::operator-= (T const& s) -> mat2s_t<T>&
{
    this->xx -= s;
    this->xy -= s;
    this->yy -= s;
    return *this;
}

template<class T>
inline constexpr auto mat2s_t<T>::operator*= (T const& s) -> mat2s_t<T>&
{
    this->xx *= s;
    this->xy *= s;
    this->yy *= s;
    return *this;
}

template<class T>
inline constexpr auto mat2s_t<T>::operator/= (T const& s) -> mat2s_t<T>&
{
    this->xx /= s;
    this->xy /= s;
    this->yy /= s;
    return *this;
}


template<class T>
inline constexpr auto mat2s_t<T>::vtmv(vec2_t<T> const& v) const -> T
{
    return v.x * v.x * this->xx
         + v.x * v.y * this->xy
         + v.y * v.x * this->xy
         + v.y * v.y * this->yy;
}

template<class T>
inline constexpr auto mat2s_t<T>::inverse(T eps) const -> std::optional<mat2s_t<T>>
{
    auto const d = this->det();

    if (std::abs(d) <= eps)
        return std::nullopt;

    return {{ this->yy / d, -this->xy / d, this->xx / d }};
}

template<class T>
inline constexpr auto mat2s_t<T>::det() const -> T
{
    return this->xx * this->yy - this->xy * this->xy;
}

template<class T>
inline constexpr auto mat2s_t<T>::trace() const -> T
{
    return this->xx + this->yy;
}


template<class T>
inline constexpr auto mat2s_t<T>::eigen(T eps) const -> eigen2_t<T>
{
    auto const [ew1, ew2] = this->eigenvalues(eps);

    return {{ ew1, ew2 }, { this->eigenvector(ew1), this->eigenvector(ew2) }};
}

template<class T>
inline constexpr auto mat2s_t<T>::eigenvalues(T eps) const -> std::array<T, 2>
{
    return solve_quadratic(num<T>::one, -this->trace(), this->det(), eps);
}

template<class T>
inline constexpr auto mat2s_t<T>::eigenvector(T eigenvalue) const -> vec2_t<T>
{
    auto ev = vec2_t<T>{};

    /*
     * This 'if' should prevent two problems:
     * 1. Cancellation due to small values in subtraction.
     * 2. The vector being { 0, 0 }.
     */
    if (std::abs(this->xx - eigenvalue) > std::abs(this->yy - eigenvalue)) {
        ev = { -this->xy, this->xx - eigenvalue };
    } else {
        ev = { this->yy - eigenvalue, -this->xy };
    }

    return ev / ev.norm_l2();
}


template<class T>
template<class S>
inline constexpr auto mat2s_t<T>::cast() const -> mat2s_t<S>
{
    return { static_cast<S>(this->xx), static_cast<S>(this->xy), static_cast<S>(this->yy) };
}


template<typename T>
auto operator<< (std::ostream& os, mat2s_t<T> const& m) -> std::ostream&
{
    return os << "[[" << m.xx << ", " << m.xy << "], [" << m.xy << ", " << m.yy << "]]";
}


template<class T>
inline constexpr auto operator+ (mat2s_t<T> const& a, mat2s_t<T> const& b) -> mat2s_t<T>
{
    return { a.xx + b.xx, a.xy + b.xy, a.yy + b.yy };
}

template<class T>
inline constexpr auto operator+ (mat2s_t<T> const& m, T const& s) -> mat2s_t<T>
{
    return { m.xx + s, m.xy + s, m.yy + s };
}

template<class T>
inline constexpr auto operator+ (T const& s, mat2s_t<T> const& m) -> mat2s_t<T>
{
    return { s + m.xx, s + m.xy, s + m.yy };
}

template<class T>
inline constexpr auto operator- (mat2s_t<T> const& a, mat2s_t<T> const& b) -> mat2s_t<T>
{
    return { a.xx - b.xx, a.xy - b.xy, a.yy - b.yy };
}

template<class T>
inline constexpr auto operator- (mat2s_t<T> const& m, T const& s) -> mat2s_t<T>
{
    return { m.xx - s, m.xy - s, m.yy - s };
}

template<class T>
inline constexpr auto operator- (T const& s, mat2s_t<T> const& m) -> mat2s_t<T>
{
    return { s - m.xx, s - m.xy, s - m.yy };
}

template<class T>
inline constexpr auto operator* (mat2s_t<T> const& m, T const& s) -> mat2s_t<T>
{
    return { m.xx * s, m.xy * s, m.yy * s };
}

template<class T>
inline constexpr auto operator* (T const& s, mat2s_t<T> const& m) -> mat2s_t<T>
{
    return { s * m.xx, s * m.xy, s * m.yy };
}

template<class T>
inline constexpr auto operator/ (mat2s_t<T> const& m, T const& s) -> mat2s_t<T>
{
    return { m.xx / s, m.xy / s, m.yy / s };
}

template<class T>
inline constexpr auto operator/ (T const& s, mat2s_t<T> const& m) -> mat2s_t<T>
{
    return { s / m.xx, s / m.xy, s / m.yy };
}


template<class T>
struct num<mat2s_t<T>> {
    static inline constexpr mat2s_t<T> zero = { num<T>::zero, num<T>::zero, num<T>::zero };
};

} /* namespace math */
