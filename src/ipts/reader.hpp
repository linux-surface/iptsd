// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_READER_HPP
#define IPTSD_IPTS_READER_HPP

#include <common/types.hpp>

#include <gsl/gsl>

namespace iptsd::ipts {

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

		auto begin = m_data.begin();
		std::advance(begin, m_index);

		auto end = begin;
		std::advance(end, dest.size());

		std::copy(begin, end, dest.begin());
		m_index += dest.size();
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
	gsl::span<u8> subspan(usize size)
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
	Reader sub(usize size)
	{
		return Reader {this->subspan(size)};
	}

	/*!
	 * Reads an object from the current position.
	 *
	 * @tparam The type (and size) of the object to read.
	 * @return The object that was read.
	 */
	template <class T> T read()
	{
		T value {};

		// We have to break type safety here, since all we have is a bytestream.
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		this->read(gsl::span {reinterpret_cast<u8 *>(&value), sizeof(value)});

		return value;
	}
};

} // namespace iptsd::ipts

#endif // IPTSD_IPTS_READER_HPP
