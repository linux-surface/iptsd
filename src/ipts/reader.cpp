// SPDX-License-Identifier: GPL-2.0-or-later

#include "reader.hpp"

#include <stdexcept>

namespace iptsd::ipts {

void Reader::read(const gsl::span<u8> dest)
{
	if (dest.size() > this->size())
		throw std::runtime_error("Tried to read more data than available!");

	auto begin = this->data.begin();
	std::advance(begin, this->index);

	auto end = begin;
	std::advance(end, dest.size());

	std::copy(begin, end, dest.begin());
	this->index += dest.size();
}

void Reader::skip(const size_t size)
{
	if (size > this->size())
		throw std::runtime_error("Tried to read more data than available!");

	this->index += size;
}

std::size_t Reader::size()
{
	return this->data.size() - this->index;
}

Reader Reader::sub(std::size_t size)
{
	if (size > this->size())
		throw std::runtime_error("Tried to read more data than available!");

	const gsl::span<u8> sub = this->data.subspan(this->index, size);
	this->skip(size);

	return Reader {sub};
}

} // namespace iptsd::ipts
