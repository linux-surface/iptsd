// SPDX-License-Identifier: GPL-2.0-or-later

#include "reader.hpp"

#include "types.hpp"

#include <cstddef>
#include <cstring>
#include <stdexcept>

IptsdReader::IptsdReader(size_t size)
{
	this->data = new u8[size];
	this->size = size;
	this->current = 0;
}

IptsdReader::~IptsdReader(void)
{
	delete[] this->data;
}

void IptsdReader::read(void *dest, size_t size)
{
	if (!dest)
		throw std::invalid_argument("Destination buffer is NULL");

	if (this->current + size > this->size)
		throw IptsdReaderException("Reading beyond data buffer limit");

	memcpy(dest, &this->data[this->current], size);
	this->current += size;
}

void IptsdReader::skip(size_t size)
{
	if (this->current + size > this->size)
		throw IptsdReaderException("Reading beyond data buffer limit");

	this->current += size;
}

void IptsdReader::reset(void)
{
	this->current = 0;
	memset(this->data, 0, this->size);
}
