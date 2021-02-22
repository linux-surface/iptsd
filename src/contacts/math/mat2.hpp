#pragma once

#include "math/num.hpp"
#include "math/poly2.hpp"
#include "math/vec2.hpp"

#include <iostream>
#include <optional>


namespace iptsd::math {

template<class T>
struct Eigen2 {
    std::array<T, 2>       w;
    std::array<Vec2<T>, 2> v;
};


template<class T>
struct Mat2s {
public:
    T xx, xy, yy;

public:
    using value_type = T;

public:
    static constexpr auto identity() -> Mat2s<T>;

    constexpr auto operator+= (Mat2s<T> const& m) -> Mat2s<T>&;
    constexpr auto operator+= (T const& s) -> Mat2s<T>&;

    constexpr auto operator-= (Mat2s<T> const& m) -> Mat2s<T>&;
    constexpr auto operator-= (T const& s) -> Mat2s<T>&;

    constexpr auto operator*= (T const& s) -> Mat2s<T>&;
    constexpr auto operator/= (T const& s) -> Mat2s<T>&;

    constexpr auto vtmv(Vec2<T> const& v) const -> T;

    constexpr auto inverse(T eps=num<T>::eps) const -> std::optional<Mat2s<T>>;

    constexpr auto det() const -> T;
    constexpr auto trace() const -> T;

    constexpr auto eigen(T eps=num<T>::eps) const -> Eigen2<T>;
    constexpr auto eigenvalues(T eps=num<T>::eps) const -> std::array<T, 2>;
    constexpr auto eigenvector(T eigenvalue) const -> Vec2<T>;

    template<class S>
    constexpr auto cast() const -> Mat2s<S>;
};


template<class T>
inline constexpr auto Mat2s<T>::identity() -> Mat2s<T>
{
    return { num<T>::one, num<T>::zero, num<T>::one };
}


template<class T>
inline constexpr auto Mat2s<T>::operator+= (Mat2s<T> const& m) -> Mat2s<T>&
{
    this->xx += m.xx;
    this->xy += m.xy;
    this->yy += m.yy;
    return *this;
}

template<class T>
inline constexpr auto Mat2s<T>::operator+= (T const& s) -> Mat2s<T>&
{
    this->xx += s;
    this->xy += s;
    this->yy += s;
    return *this;
}

template<class T>
inline constexpr auto Mat2s<T>::operator-= (Mat2s<T> const& m) -> Mat2s<T>&
{
    this->xx -= m.xx;
    this->xy -= m.xy;
    this->yy -= m.yy;
    return *this;
}

template<class T>
inline constexpr auto Mat2s<T>::operator-= (T const& s) -> Mat2s<T>&
{
    this->xx -= s;
    this->xy -= s;
    this->yy -= s;
    return *this;
}

template<class T>
inline constexpr auto Mat2s<T>::operator*= (T const& s) -> Mat2s<T>&
{
    this->xx *= s;
    this->xy *= s;
    this->yy *= s;
    return *this;
}

template<class T>
inline constexpr auto Mat2s<T>::operator/= (T const& s) -> Mat2s<T>&
{
    this->xx /= s;
    this->xy /= s;
    this->yy /= s;
    return *this;
}


template<class T>
inline constexpr auto Mat2s<T>::vtmv(Vec2<T> const& v) const -> T
{
    return v.x * v.x * this->xx
         + v.x * v.y * this->xy
         + v.y * v.x * this->xy
         + v.y * v.y * this->yy;
}

template<class T>
inline constexpr auto Mat2s<T>::inverse(T eps) const -> std::optional<Mat2s<T>>
{
    auto const d = this->det();

    if (std::abs(d) <= eps)
        return std::nullopt;

    return {{ this->yy / d, -this->xy / d, this->xx / d }};
}

template<class T>
inline constexpr auto Mat2s<T>::det() const -> T
{
    return this->xx * this->yy - this->xy * this->xy;
}

template<class T>
inline constexpr auto Mat2s<T>::trace() const -> T
{
    return this->xx + this->yy;
}


template<class T>
inline constexpr auto Mat2s<T>::eigen(T eps) const -> Eigen2<T>
{
    auto const [ew1, ew2] = this->eigenvalues(eps);

    return {{ ew1, ew2 }, { this->eigenvector(ew1), this->eigenvector(ew2) }};
}

template<class T>
inline constexpr auto Mat2s<T>::eigenvalues(T eps) const -> std::array<T, 2>
{
    return solve_quadratic(num<T>::one, -this->trace(), this->det(), eps);
}

template<class T>
inline constexpr auto Mat2s<T>::eigenvector(T eigenvalue) const -> Vec2<T>
{
    auto ev = Vec2<T>{};

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
inline constexpr auto Mat2s<T>::cast() const -> Mat2s<S>
{
    return { static_cast<S>(this->xx), static_cast<S>(this->xy), static_cast<S>(this->yy) };
}


template<typename T>
auto operator<< (std::ostream& os, Mat2s<T> const& m) -> std::ostream&
{
    return os << "[[" << m.xx << ", " << m.xy << "], [" << m.xy << ", " << m.yy << "]]";
}


template<class T>
inline constexpr auto operator+ (Mat2s<T> const& a, Mat2s<T> const& b) -> Mat2s<T>
{
    return { a.xx + b.xx, a.xy + b.xy, a.yy + b.yy };
}

template<class T>
inline constexpr auto operator+ (Mat2s<T> const& m, T const& s) -> Mat2s<T>
{
    return { m.xx + s, m.xy + s, m.yy + s };
}

template<class T>
inline constexpr auto operator+ (T const& s, Mat2s<T> const& m) -> Mat2s<T>
{
    return { s + m.xx, s + m.xy, s + m.yy };
}

template<class T>
inline constexpr auto operator- (Mat2s<T> const& a, Mat2s<T> const& b) -> Mat2s<T>
{
    return { a.xx - b.xx, a.xy - b.xy, a.yy - b.yy };
}

template<class T>
inline constexpr auto operator- (Mat2s<T> const& m, T const& s) -> Mat2s<T>
{
    return { m.xx - s, m.xy - s, m.yy - s };
}

template<class T>
inline constexpr auto operator- (T const& s, Mat2s<T> const& m) -> Mat2s<T>
{
    return { s - m.xx, s - m.xy, s - m.yy };
}

template<class T>
inline constexpr auto operator* (Mat2s<T> const& m, T const& s) -> Mat2s<T>
{
    return { m.xx * s, m.xy * s, m.yy * s };
}

template<class T>
inline constexpr auto operator* (T const& s, Mat2s<T> const& m) -> Mat2s<T>
{
    return { s * m.xx, s * m.xy, s * m.yy };
}

template<class T>
inline constexpr auto operator/ (Mat2s<T> const& m, T const& s) -> Mat2s<T>
{
    return { m.xx / s, m.xy / s, m.yy / s };
}

template<class T>
inline constexpr auto operator/ (T const& s, Mat2s<T> const& m) -> Mat2s<T>
{
    return { s / m.xx, s / m.xy, s / m.yy };
}


template<class T>
struct num<Mat2s<T>> {
    static inline constexpr Mat2s<T> zero = { num<T>::zero, num<T>::zero, num<T>::zero };
};

} /* namespace iptsd::math */


/* imports */
namespace iptsd {

using math::Mat2s;

} /* namespace iptsd */
