#pragma once

#include "types.hpp"


namespace gfx {

struct srgb {
    f32 r, g, b;

    static constexpr auto from(f32 r, f32 g, f32 b) -> srgb;
};

struct srgba {
    f32 r, g, b, a;

    static constexpr auto from(f32 r, f32 g, f32 b) -> srgba;
};


constexpr auto srgb::from(f32 r, f32 g, f32 b) -> srgb
{
    return {r, g, b};
}

constexpr auto operator+(srgb a, srgb b) -> srgb
{
    return {a.r + b.r, a.g + b.g, a.b + b.b};
}

constexpr auto operator*(f32 s, srgb c) -> srgb
{
    return {s * c.r, s * c.g, s * c.b};
}

constexpr auto operator*(srgb c, f32 s) -> srgb
{
    return s * c;
}


constexpr auto srgba::from(f32 r, f32 g, f32 b) -> srgba
{
    return {r, g, b, 1.0};
}

} /* namespace gfx */
