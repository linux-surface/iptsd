#pragma once

#include "math/num.hpp"

#include <array>
#include <cmath>


namespace math {

template<class T>
auto solve_quadratic(T a, T b, T c, T eps=num<T>::eps) -> std::array<T, 2>
{
    if (std::abs(a) <= eps) {           // case: bx + c = 0
        return { -c / b, num<T>::zero };
    }

    if (std::abs(c) <= eps) {           // case: ax^2 + bx = 0
        return { -b / a, num<T>::zero };
    }

    // Note: Does not prevent potential overflows in b^2

    // stable(-ish) algorithm: prevent cancellation
    auto const r1 = (-b - std::copysign(std::sqrt(b * b - 4 * a * c), b)) / (2 * a);
    auto const r2 = c / (a * r1);

    return { r1, r2 };
}

} /* namespace math */
