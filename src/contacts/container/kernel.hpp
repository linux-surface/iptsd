#pragma once

#include "types.hpp"
#include "utils/access.hpp"

#include <array>
#include <iostream>


namespace iptsd::container {

template<class T, index_t Nx, index_t Ny>
struct Kernel {
public:
    std::array<T, Nx * Ny> buf;

public:
    using value_type             = T;
    using reference              = value_type&;
    using const_reference        = value_type const&;
    using pointer                = value_type*;
    using const_pointer          = value_type const*;
    using iterator               = T*;
    using const_iterator         = T const*;
    using reverse_iterator       = T*;
    using const_reverse_iterator = T const*;

public:
    template<class... Args>
    constexpr Kernel(Args... args);

    constexpr Kernel(Kernel<T, Nx, Ny> const& other);

    constexpr auto operator= (Kernel<T, Nx, Ny> const& rhs) -> Kernel<T, Nx, Ny>&;

    auto size() const -> index2_t;
    auto stride() const -> index_t;

    auto data() -> pointer;
    auto data() const -> const_pointer;

    auto operator[] (index2_t const& i) const -> const_reference;
    auto operator[] (index2_t const& i) -> reference;

    auto operator[] (index_t const& i) const -> const_reference;
    auto operator[] (index_t const& i) -> reference;

    auto begin() -> iterator;
    auto end() -> iterator;

    auto begin() const -> const_iterator;
    auto end() const -> const_iterator;

    auto cbegin() const -> const_iterator;
    auto cend() const -> const_iterator;

    static constexpr auto ravel(index2_t size, index2_t i) -> index_t;
    static constexpr auto unravel(index2_t size, index_t i) -> index2_t;
};

template<class T, index_t Nx, index_t Ny>
template<class... Args>
inline constexpr Kernel<T, Nx, Ny>::Kernel(Args... args)
    : buf{std::forward<Args>(args)...}
{}


template<class T, index_t Nx, index_t Ny>
inline constexpr Kernel<T, Nx, Ny>::Kernel(Kernel<T, Nx, Ny> const& other)
    : buf{other.buf}
{}

template<class T, index_t Nx, index_t Ny>
inline constexpr auto Kernel<T, Nx, Ny>::operator= (Kernel<T, Nx, Ny> const& rhs) -> Kernel<T, Nx, Ny>&
{
    this->buf = rhs.buf;
    return *this;
}

template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::size() const -> index2_t
{
    return { Nx, Ny };
}

template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::stride() const -> index_t
{
    return Nx;
}

template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::data() -> pointer
{
    return this->buf.data();
}
template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::data() const -> const_pointer
{
    return this->buf.data();
}

template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::operator[] (index2_t const& i) const -> const_reference
{
    return utils::access::access<T>(this->buf, ravel, { Nx, Ny }, i);
}
template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::operator[] (index2_t const& i) -> reference
{
    return utils::access::access<T>(this->buf, ravel, { Nx, Ny }, i);
}

template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::operator[] (index_t const& i) const -> const_reference
{
    return utils::access::access<T>(this->buf, Nx * Ny, i);
}
template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::operator[] (index_t const& i) -> reference
{
    return utils::access::access<T>(this->buf, Nx * Ny, i);
}

template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::begin() -> iterator
{
    return &this->buf[0];
}
template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::end() -> iterator
{
    return &this->buf[Nx * Ny];
}

template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::begin() const -> const_iterator
{
    return &this->buf[0];
}
template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::end() const -> const_iterator
{
    return &this->buf[Nx * Ny];
}

template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::cbegin() const -> const_iterator
{
    return &this->buf[0];
}

template<class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::cend() const -> const_iterator
{
    return &this->buf[Nx * Ny];
}


template<class T, index_t Nx, index_t Ny>
auto operator<< (std::ostream& os, Kernel<T, Nx, Ny> const& k) -> std::ostream&
{
    os << "[[" << k[{0, 0}];

    for (index_t x = 1; x < Nx; ++x) {
        os << ", " << k[{x, 0}];
    }

    for (index_t y = 1; y < Ny; ++y) {
        os << "], [" << k[{0, y}];

        for (index_t x = 1; x < Nx; ++x) {
            os << ", " << k[{x, y}];
        }
    }

    return os << "]]";
}


template<class T, index_t Nx, index_t Ny>
inline constexpr auto Kernel<T, Nx, Ny>::ravel(index2_t size, index2_t i) -> index_t
{
    return i.y * size.x + i.x;
}

template<class T, index_t Nx, index_t Ny>
inline constexpr auto Kernel<T, Nx, Ny>::unravel(index2_t size, index_t i) -> index2_t
{
    return { i % size.x, i / size.x };
}

} /* namespace iptsd::container */


/* imports */
namespace iptsd {

using container::Kernel;

} /* namespace iptsd */
