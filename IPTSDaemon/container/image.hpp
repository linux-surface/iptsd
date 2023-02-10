/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTAINER_IMAGE_HPP
#define IPTSD_CONTAINER_IMAGE_HPP

#include <common/types.hpp>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

namespace iptsd::container {

template <class T> class Image {
public:
	using array_type = std::vector<T>;
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
	Image();
	Image(index2_t size);

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

private:
	index2_t m_size;
	array_type m_data {};
};

template <class T> Image<T>::Image() : m_size {0, 0}
{
}

template <class T> Image<T>::Image(index2_t size) : m_size {size}, m_data(size.span())
{
}

template <class T> inline auto Image<T>::size() const -> index2_t
{
	return m_size;
}

template <class T> inline auto Image<T>::stride() const -> index_t
{
	return m_size.x;
}

template <class T> inline auto Image<T>::data() -> pointer
{
	return m_data.data();
}

template <class T> inline auto Image<T>::data() const -> const_pointer
{
	return m_data.data();
}

template <class T> inline auto Image<T>::operator[](index2_t const &i) const -> const_reference
{
	return m_data[ravel(m_size, i)];
}

template <class T> inline auto Image<T>::operator[](index2_t const &i) -> reference
{
	return m_data[ravel(m_size, i)];
}

template <class T> inline auto Image<T>::operator[](index_t const &i) const -> const_reference
{
	return m_data[i];
}

template <class T> inline auto Image<T>::operator[](index_t const &i) -> reference
{
	return m_data[i];
}

template <class T> inline auto Image<T>::begin() -> iterator
{
	return m_data.begin();
}

template <class T> inline auto Image<T>::end() -> iterator
{
	return m_data.end();
}

template <class T> inline auto Image<T>::begin() const -> const_iterator
{
	return m_data.cbegin();
}

template <class T> inline auto Image<T>::end() const -> const_iterator
{
	return m_data.cend();
}

template <class T> inline auto Image<T>::cbegin() const -> const_iterator
{
	return m_data.cbegin();
}

template <class T> inline auto Image<T>::cend() const -> const_iterator
{
	return m_data.cend();
}

template <class T> inline constexpr auto Image<T>::ravel(index2_t size, index2_t i) -> index_t
{
	return i.y * size.x + i.x;
}

template <class T> inline constexpr auto Image<T>::unravel(index2_t size, index_t i) -> index2_t
{
	return {i % size.x, i / size.x};
}

} /* namespace iptsd::container */

#endif /* IPTSD_CONTAINER_IMAGE_HPP */
