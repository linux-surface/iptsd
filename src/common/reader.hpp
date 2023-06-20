// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_READER_HPP
#define IPTSD_COMMON_READER_HPP

#include "types.hpp"

#include <gsl/gsl>

#include <algorithm>

namespace iptsd {

class Reader {
private:
	gsl::span<u8> m_data;

	// The current position in the data.
	usize m_index = 0;

public:
	Reader(const gsl::span<u8> data) : m_data {data} {};

	/*!
	 * Fills a buffer with the data at the current position.
	 *
	 * @param[in] dest The destination and size of the data.
	 */
	void read(const gsl::span<u8> dest)
	{
		if (dest.size() > this->size())
			throw std::runtime_error("Tried to read more data than available!");

		const gsl::span<u8> src = this->subspan(dest.size());
		std::copy(src.begin(), src.end(), dest.begin());
	}

	/*!
	 * Moves the current position forward.
	 *
	 * @param[in] size How many bytes to skip.
	 */
	void skip(const usize size)
	{
		if (size > this->size())
			throw std::runtime_error("Tried to read more data than available!");

		m_index += size;
	}

	/*!
	 * How many bytes are left in the data.
	 *
	 * @return The amount of bytes that have not been read.
	 */
	[[nodiscard]] usize size() const
	{
		return m_data.size() - m_index;
	}

	/*!
	 * Takes a chunk of bytes from the current position and splits it off.
	 *
	 * @param[in] size How many bytes to take.
	 * @return The raw chunk of data.
	 */
	gsl::span<u8> subspan(const usize size)
	{
		if (size > this->size())
			throw std::runtime_error("Tried to read more data than available!");

		const gsl::span<u8> sub = m_data.subspan(m_index, size);
		this->skip(size);

		return sub;
	}

	/*!
	 * Takes a chunk of bytes from the current position and splits it off.
	 *
	 * @param[in] size How many bytes to take.
	 * @return A new reader instance for the chunk of data.
	 */
	Reader sub(const usize size)
	{
		return Reader {this->subspan(size)};
	}

	/*!
	 * Reads an object from the current position.
	 *
	 * @tparam The type (and size) of the object to read.
	 * @return The object that was read.
	 */
	template <class T>
	T read()
	{
		T value {};

		// We have to break type safety here, since all we have is a bytestream.
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		this->read(gsl::span {reinterpret_cast<u8 *>(&value), sizeof(value)});

		return value;
	}
};

} // namespace iptsd

#endif // IPTSD_COMMON_READER_HPP
