// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_DEVICE_FILE_HPP
#define IPTSD_CORE_LINUX_DEVICE_FILE_HPP

#include "errors.hpp"

#include <common/casts.hpp>
#include <common/error.hpp>
#include <common/file.hpp>
#include <common/reader.hpp>
#include <common/types.hpp>
#include <hid/device.hpp>
#include <hid/parser.hpp>
#include <hid/report.hpp>
#include <ipts/parser.hpp>

#include <gsl/gsl>

#include <linux/hidraw.h>

#include <filesystem>

namespace iptsd::core::linux::device {

class File : public hid::Device {
protected:
	Reader m_data;
	std::filesystem::path m_path {};

	// The index at which the actual data starts.
	usize m_start = 0;

	struct hidraw_devinfo m_devinfo {};
	struct hidraw_report_descriptor m_desc {};

public:
	File(const std::filesystem::path &path)
		: m_data {common::read_all_bytes(path)},
		  m_path {path}
	{
		m_devinfo = m_data.read<struct hidraw_devinfo>();
		m_desc.size = m_data.read<u32>();
		m_data.read(gsl::span<u8> {&m_desc.value[0], m_desc.size});

		/*
		 * We have to get the index after reading the header, so this actually
		 * has to happen here, even if clang-tidy thinks it is smarter.
		 */
		m_start = m_data.index(); // NOLINT(cppcoreguidelines-prefer-member-initializer)
	}

	/*!
	 * The "name", aka. the path to the source file.
	 */
	std::string_view name() override
	{
		return m_path.c_str();
	}

	/*!
	 * The vendor ID of the device.
	 */
	u16 vendor() override
	{
		/*
		 * The value is just an ID stored in a signed value.
		 * A negative device ID doesn't make sense, so cast it away.
		 */
		return gsl::narrow_cast<u16>(m_devinfo.vendor);
	}

	/*!
	 * The product ID of the device.
	 */
	u16 product() override
	{
		/*
		 * The value is just an ID stored in a signed value.
		 * A negative device ID doesn't make sense, so cast it away.
		 */
		return gsl::narrow_cast<u16>(m_devinfo.product);
	}

	/*!
	 * The binary HID descriptor of the device.
	 */
	gsl::span<u8> raw_descriptor() override
	{
		return gsl::span<u8> {&m_desc.value[0], m_desc.size};
	}

	/*!
	 * Reads a report from the stored HID data.
	 *
	 * @param[in] buffer The target storage for the report.
	 * @return The size of the report that was read in bytes.
	 */
	usize read(gsl::span<u8> buffer) override
	{
		try {
			const auto size = casts::to<usize>(m_data.read<u64>());
			m_data.read(buffer.first(size));
			return size;
		} catch (const common::Error<Reader::Error::EndOfData> & /* unused */) {
			// Allow looping calls to the file based HID source
			m_data.seek(m_start);
			throw common::Error<Error::EndOfData> {};
		}
	}

	/*!
	 * Reads the value of a HID feature report from the stored data.
	 *
	 * @param[in] report The report ID to get, followed by enough space to fit the data.
	 */
	void get_feature(gsl::span<u8> report) override
	{
		try {
			const auto size = casts::to<usize>(m_data.read<u64>());
			m_data.read(report.first(size));
		} catch (const common::Error<Reader::Error::EndOfData> & /* unused */) {
			// Allow looping calls to the file based HID source
			m_data.seek(m_start);
			throw common::Error<Error::EndOfData> {};
		}
	}

	/*!
	 * Sets the value of a HID feature report.
	 *
	 * This function is not implemented.
	 *
	 * Setting features is serialized implicitly, because the behaviour of the device and
	 * therefor the following data will change.
	 *
	 * @param[in] report The report ID to set, followed by the new data.
	 */
	void set_feature(const gsl::span<u8> /* unused */) override
	{
	}
};

} // namespace iptsd::core::linux::device

#endif // IPTSD_CORE_LINUX_DEVICE_FILE_HPP
