#pragma once

#include "types.hpp"


namespace iptsd::gfx {

struct Srgb {
    f32 r, g, b;

    static constexpr auto from(f32 r, f32 g, f32 b) -> Srgb;
};

struct Srgba {
    f32 r, g, b, a;

    static constexpr auto from(f32 r, f32 g, f32 b) -> Srgba;
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

} /* namespace iptsd::gfx */
