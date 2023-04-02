// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_CHRONO_HPP
#define IPTSD_COMMON_CHRONO_HPP

#include "types.hpp"

#include <chrono>
#include <ratio>

namespace chrono = std::chrono;

// NOLINTBEGIN(misc-unused-using-decls)
using chrono::operator""h;
using chrono::operator""min;
using chrono::operator""s;
using chrono::operator""ms;
using chrono::operator""us;
using chrono::operator""ns;
// NOLINTEND(misc-unused-using-decls)

template <class T>
using hours = chrono::duration<T, std::ratio<3600>>;

template <class T>
using minutes = chrono::duration<T, std::ratio<60>>;

template <class T>
using seconds = chrono::duration<T, std::ratio<1>>;

template <class T>
using milliseconds = chrono::duration<T, std::milli>;

template <class T>
using microseconds = chrono::duration<T, std::micro>;

template <class T>
using nanoseconds = chrono::duration<T, std::nano>;

#endif // IPTSD_COMMON_CHRONO_HPP
