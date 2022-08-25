/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTAINER_KERNEL_HPP
#define IPTSD_CONTAINER_KERNEL_HPP

#include <common/types.hpp>

#include <array>
#include <iostream>

namespace iptsd::container {

template <class T, index_t Nx, index_t Ny> struct Kernel {
public:
	using array_type = std::array<T, static_cast<std::size_t>(Nx) * Ny>;
	using iterator = typename array_type::iterator;
	using const_iterator = typename array_type::const_iterator;
	using reverse_iterator = typename array_type::reverse_iterator;
	using const_reverse_iterator = typename array_type::const_reverse_iterator;

	using value_type = typename array_type::value_type;
	using reference = typename array_type::reference;
	using const_reference = typename array_type::const_reference;
	using pointer = typename array_type::pointer;
	using const_pointer = typename array_type::const_pointer;

public:
	array_type buf;

public:
	template <class... Args> constexpr Kernel(Args... args);

	constexpr Kernel(Kernel<T, Nx, Ny> const &other);

	constexpr auto operator=(Kernel<T, Nx, Ny> const &rhs) -> Kernel<T, Nx, Ny> &;

	[[nodiscard]] auto size() const -> index2_t;
	[[nodiscard]] auto stride() const -> index_t;

	auto data() -> pointer;
	[[nodiscard]] auto data() const -> const_pointer;

	auto operator[](index2_t const &i) const -> const_reference;
	auto operator[](index2_t const &i) -> reference;

	auto operator[](index_t const &i) const -> const_reference;
	auto operator[](index_t const &i) -> reference;

	auto begin() -> iterator;
	auto end() -> iterator;

	[[nodiscard]] auto begin() const -> const_iterator;
	[[nodiscard]] auto end() const -> const_iterator;

	[[nodiscard]] auto cbegin() const -> const_iterator;
	[[nodiscard]] auto cend() const -> const_iterator;

	static constexpr auto ravel(index2_t size, index2_t i) -> index_t;
	static constexpr auto unravel(index2_t size, index_t i) -> index2_t;
};

template <class T, index_t Nx, index_t Ny>
template <class... Args>
inline constexpr Kernel<T, Nx, Ny>::Kernel(Args... args) : buf {std::forward<Args>(args)...}
{
}

template <class T, index_t Nx, index_t Ny>
inline constexpr Kernel<T, Nx, Ny>::Kernel(Kernel<T, Nx, Ny> const &other) : buf {other.buf}
{
}

template <class T, index_t Nx, index_t Ny>
inline constexpr auto Kernel<T, Nx, Ny>::operator=(Kernel<T, Nx, Ny> const &rhs)
	-> Kernel<T, Nx, Ny> &
{
	if (this != &rhs)
		this->buf = rhs.buf;

	return *this;
}

template <class T, index_t Nx, index_t Ny> auto Kernel<T, Nx, Ny>::size() const -> index2_t
{
	return {Nx, Ny};
}

template <class T, index_t Nx, index_t Ny> auto Kernel<T, Nx, Ny>::stride() const -> index_t
{
	return Nx;
}

template <class T, index_t Nx, index_t Ny> auto Kernel<T, Nx, Ny>::data() -> pointer
{
	return this->buf.data();
}
template <class T, index_t Nx, index_t Ny> auto Kernel<T, Nx, Ny>::data() const -> const_pointer
{
	return this->buf.data();
}

template <class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::operator[](index2_t const &i) const -> const_reference
{
	return this->buf[ravel({Nx, Ny}, i)];
}

template <class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::operator[](index2_t const &i) -> reference
{
	return this->buf[ravel({Nx, Ny}, i)];
}

template <class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::operator[](index_t const &i) const -> const_reference
{
	return this->buf[i];
}

template <class T, index_t Nx, index_t Ny>
auto Kernel<T, Nx, Ny>::operator[](index_t const &i) -> reference
{
	return this->buf[i];
}

template <class T, index_t Nx, index_t Ny> auto Kernel<T, Nx, Ny>::begin() -> iterator
{
	return this->buf.begin();
}

template <class T, index_t Nx, index_t Ny> auto Kernel<T, Nx, Ny>::end() -> iterator
{
	return this->buf.end();
}

template <class T, index_t Nx, index_t Ny> auto Kernel<T, Nx, Ny>::begin() const -> const_iterator
{
	return this->buf.begin();
}

template <class T, index_t Nx, index_t Ny> auto Kernel<T, Nx, Ny>::end() const -> const_iterator
{
	return this->buf.end();
}

template <class T, index_t Nx, index_t Ny> auto Kernel<T, Nx, Ny>::cbegin() const -> const_iterator
{
	return this->buf.cbegin();
}

template <class T, index_t Nx, index_t Ny> auto Kernel<T, Nx, Ny>::cend() const -> const_iterator
{
	return this->buf.cend();
}

template <class T, index_t Nx, index_t Ny>
auto operator<<(std::ostream &os, Kernel<T, Nx, Ny> const &k) -> std::ostream &
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

template <class T, index_t Nx, index_t Ny>
inline constexpr auto Kernel<T, Nx, Ny>::ravel(index2_t size, index2_t i) -> index_t
{
	return i.y * size.x + i.x;
}

template <class T, index_t Nx, index_t Ny>
inline constexpr auto Kernel<T, Nx, Ny>::unravel(index2_t size, index_t i) -> index2_t
{
	return {i % size.x, i / size.x};
}

} /* namespace iptsd::container */

#endif /* IPTSD_CONTAINER_IMAGE_HPP */
