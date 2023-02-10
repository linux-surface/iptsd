/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_GFX_COLOR_HPP
#define IPTSD_GFX_COLOR_HPP

#include <common/types.hpp>

#include <gsl/gsl>

namespace iptsd::gfx {

struct Srgb {
	f32 r, g, b;

	static constexpr auto from(f32 r, f32 g, f32 b) -> Srgb;
};

struct Srgba {
	f32 r, g, b, a;

	static constexpr auto from(f32 r, f32 g, f32 b) -> Srgba;
};

struct Argb {
	u32 color;

	static constexpr auto from(f32 r, f32 g, f32 b) -> Argb;
};

constexpr auto Srgb::from(f32 r, f32 g, f32 b) -> Srgb
{
	return {r, g, b};
}

constexpr auto operator+(Srgb a, Srgb b) -> Srgb
{
	return {a.r + b.r, a.g + b.g, a.b + b.b};
}

constexpr auto operator*(f32 s, Srgb c) -> Srgb
{
	return {s * c.r, s * c.g, s * c.b};
}

constexpr auto operator*(Srgb c, f32 s) -> Srgb
{
	return s * c;
}

constexpr auto Srgba::from(f32 r, f32 g, f32 b) -> Srgba
{
	return {r, g, b, 1.0};
}

constexpr auto Argb::from(f32 r, f32 g, f32 b) -> Argb
{
	u32 color = 255 << 24;
	color += gsl::narrow<u8>(std::round(r * 255)) << 16;
	color += gsl::narrow<u8>(std::round(g * 255)) << 8;
	color += gsl::narrow<u8>(std::round(b * 255));

	return Argb {color};
}

} /* namespace iptsd::gfx */

#endif /* IPTSD_GFX_COLOR_HPP */
