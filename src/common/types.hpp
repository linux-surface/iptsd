/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_COMMON_TYPES_HPP
#define IPTSD_COMMON_TYPES_HPP

#include <cmath>
#include <cstdint>
#include <iostream>

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using f32 = std::float_t;
using f64 = std::double_t;

using index_t = int;

struct index2_t {
public:
	index_t x, y;

public:
	constexpr index2_t &operator+=(index2_t const &v);
	constexpr index2_t &operator-=(index2_t const &v);

	[[nodiscard]] constexpr index_t span() const;
};

inline constexpr index2_t &index2_t::operator+=(index2_t const &v)
{
	this->x += v.x;
	this->y += v.y;
	return *this;
}

inline constexpr index2_t &index2_t::operator-=(index2_t const &v)
{
	this->x -= v.x;
	this->y -= v.y;
	return *this;
}

inline constexpr index_t index2_t::span() const
{
	return this->x * this->y;
}

inline std::ostream &operator<<(std::ostream &os, index2_t const &i)
{
	return os << "[" << i.x << ", " << i.y << "]";
}

inline constexpr bool operator==(index2_t const &a, index2_t const &b)
{
	return a.x == b.x && a.y == b.y;
}

inline constexpr bool operator!=(index2_t const &a, index2_t const &b)
{
	return !(a == b);
}

inline constexpr bool operator>(index2_t const &a, index2_t const &b)
{
	return a.x > b.x && a.y > b.y;
}

inline constexpr bool operator>=(index2_t const &a, index2_t const &b)
{
	return a.x >= b.x && a.y >= b.y;
}

inline constexpr bool operator<(index2_t const &a, index2_t const &b)
{
	return a.x < b.x && a.y < b.y;
}

inline constexpr bool operator<=(index2_t const &a, index2_t const &b)
{
	return a.x <= b.x && a.y <= b.y;
}

inline constexpr index2_t operator+(index2_t const &a, index2_t const &b)
{
	return {a.x + b.x, a.y + b.y};
}

inline constexpr index2_t operator-(index2_t const &a, index2_t const &b)
{
	return {a.x - b.x, a.y - b.y};
}

#endif /* IPTSD_COMMON_TYPES_HPP */
