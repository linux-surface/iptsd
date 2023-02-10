/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_IPTS_READER_HPP
#define IPTSD_IPTS_READER_HPP

#include <common/types.hpp>

#include <cstddef>
#include <functional>
#include <gsl/gsl>
#include <memory>
#include <vector>

namespace iptsd::ipts {

class Reader {
private:
	const gsl::span<u8> data;
	std::size_t index = 0;

public:
	Reader(const gsl::span<u8> data) : data(data) {};

	void read(const gsl::span<u8> dest);
	void skip(const size_t size);
	std::size_t size();
	Reader sub(std::size_t size);

	template <class T> T read();
};

template <class T> inline T Reader::read()
{
	T value {};

	// We have to break type safety here, since all we have is a bytestream.
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	this->read(gsl::span(reinterpret_cast<u8 *>(&value), sizeof(value)));

	return value;
}

} /* namespace iptsd::ipts */

#endif /* IPTSD_IPTS_READER_HPP */
