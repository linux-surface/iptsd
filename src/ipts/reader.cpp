// SPDX-License-Identifier: GPL-2.0-or-later

#include "reader.hpp"

namespace iptsd::ipts {

void Reader::read(const gsl::span<u8> dest)
{
	auto begin = this->data.begin();
	std::advance(begin, this->index);

	auto end = begin;
	std::advance(end, dest.size());

	std::copy(begin, end, dest.begin());
	this->index += dest.size();
}

void Reader::skip(const size_t size)
{
	this->index += size;
}

std::size_t Reader::size()
{
	return this->data.size() - this->index;
}

} // namespace iptsd::ipts
