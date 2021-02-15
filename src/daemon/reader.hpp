/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DAEMON_READER_HPP_
#define _IPTSD_DAEMON_READER_HPP_

#include <common/types.hpp>

#include <cstddef>
#include <stdexcept>

class IptsdReaderException : public std::runtime_error {
public:
	IptsdReaderException(const char *msg) : std::runtime_error(msg){};
};

class IptsdReader {
public:
	u8 *data;
	size_t size;
	size_t current;

	IptsdReader(size_t size);
	~IptsdReader(void);

	void read(void *dest, size_t size);
	void skip(size_t size);
	void reset(void);

	template <typename T> T read(void)
	{
		T value;
		this->read(&value, sizeof(value));
		return value;
	}
};

#endif /* _IPTSD_DAEMON_READER_HPP_ */
