// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_HIDRAW_DEVICE_HPP
#define IPTSD_CORE_LINUX_HIDRAW_DEVICE_HPP

#include "syscalls.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>
#include <hid/device.hpp>
#include <hid/parser.hpp>
#include <hid/report.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.hpp>

#include <gsl/gsl>

#include <linux/hidraw.h>

#include <algorithm>
#include <filesystem>
#include <optional>
#include <set>
#include <vector>

namespace iptsd::core::linux {

class HidrawDevice : public hid::Device {
private:
	int m_fd = -1;

	struct hidraw_devinfo m_devinfo {};
	struct hidraw_report_descriptor m_desc {};

	std::vector<hid::Report> m_reports {};

public:
	HidrawDevice(const std::filesystem::path &path) : m_fd {syscalls::open(path, O_RDWR)}
	{
		u32 desc_size = 0;

		syscalls::ioctl(m_fd, HIDIOCGRAWINFO, &m_devinfo);
		syscalls::ioctl(m_fd, HIDIOCGRDESCSIZE, &desc_size);

		m_desc.size = desc_size;
		syscalls::ioctl(m_fd, HIDIOCGRDESC, &m_desc);

		hid::parse(gsl::span<u8> {&m_desc.value[0], desc_size}, m_reports);
	}

	~HidrawDevice() override
	{
		try {
			syscalls::close(m_fd);
		} catch (std::exception &) {
			// ignored
		}
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
	 * The HID descriptor of the device.
	 */
	const std::vector<hid::Report> &descriptor() override
	{
		return m_reports;
	}

	/*!
	 * Reads a report from the HID device.
	 *
	 * @param[in] buffer The target storage for the report.
	 * @return The size of the report that was read in bytes.
	 */
	isize read(gsl::span<u8> buffer) override
	{
		return syscalls::read(m_fd, buffer);
	}

	/*!
	 * Gets the data of a HID feature report.
	 *
	 * @param[in] report The report ID to get, followed by enough space to fit the data.
	 */
	void get_feature(gsl::span<u8> report) override
	{
		syscalls::ioctl(m_fd, HIDIOCGFEATURE(report.size()), report.data());
	}

	/*!
	 * Sets the data of a HID feature report.
	 *
	 * @param[in] report The report ID to set, followed by the new data.
	 */
	void set_feature(const gsl::span<u8> report) override
	{
		syscalls::ioctl(m_fd, HIDIOCSFEATURE(report.size()), report.data());
	}
};

} // namespace iptsd::core::linux

#endif // IPTSD_CORE_LINUX_HIDRAW_DEVICE_HPP
