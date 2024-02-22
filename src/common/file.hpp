// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_FILE_HPP
#define IPTSD_COMMON_FILE_HPP

#include "casts.hpp"
#include "types.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

namespace iptsd::common {

/*!
 * Reads the content of a file into a byte vector.
 *
 * @param[in] path The path to the file that will be read.
 * @param[in,out] data The vector that the file will be read into.
 */
inline void read_all_bytes_into(const std::filesystem::path &path, std::vector<u8> &data)
{
	std::ifstream ifs {};
	ifs.open(path, std::ios::in | std::ios::binary);

	std::noskipws(ifs);
	data.assign(std::istream_iterator<u8>(ifs), std::istream_iterator<u8>());
}

/*!
 * Reads the content of a file into a byte vector.
 *
 * @param[in] path The path to the file that will be read.
 * @return A vector containing the entire file as bytes.
 */
inline std::vector<u8> read_all_bytes(const std::filesystem::path &path)
{
	std::vector<u8> data {};
	read_all_bytes_into(path, data);
	return data;
}

/*!
 * Serializes the contents of a span to a stream.
 *
 * @param[in] stream The stream that the data will be written to.
 * @param[in] data The span containing the data to write.
 * @tparam T The type of each element inside the span.
 */
template <class T>
inline void write_to_stream(std::ostream &stream, const gsl::span<T> data)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	stream.write(reinterpret_cast<const char *>(data.data()),
	             casts::to<std::streamsize>(data.size_bytes()));
}

/*!
 * Serializes a value to a stream.
 *
 * @param[in] stream The stream that the value will be written to.
 * @param[in] data The value that will be written to the stream.
 * @tparam T The type of the value that will be written to the stream.
 */
template <class T>
inline void write_to_stream(std::ostream &stream, const T &value)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

}; // namespace iptsd::common

#endif // IPTSD_COMMON_FILE_HPP
