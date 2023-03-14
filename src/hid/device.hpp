// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_DEVICE_HPP
#define IPTSD_HID_DEVICE_HPP

#include "descriptor.hpp"

#include <common/cerror.hpp>
#include <common/cwrap.hpp>
#include <common/types.hpp>

#include <gsl/gsl>
#include <linux/hidraw.h>
#include <string>
#include <vector>

namespace iptsd::hid {

class Device {
private:
	int m_fd = -1;
	struct hidraw_devinfo m_devinfo {};

	// The HID descriptor of the device.
	Descriptor m_desc {};

public:
	Device(const std::string &path)
	{
		int ret = common::open(path, O_RDWR);
		if (ret == -1)
			throw common::cerror("Failed to open hidraw device");

		m_fd = ret;

		ret = common::ioctl(m_fd, HIDIOCGRAWINFO, &m_devinfo);
		if (ret == -1)
			throw common::cerror("Failed to read HID device info");

		u32 desc_size = 0;

		ret = common::ioctl(m_fd, HIDIOCGRDESCSIZE, &desc_size);
		if (ret == -1)
			throw common::cerror("Failed to read HID descriptor size");

		struct hidraw_report_descriptor hidraw_desc {};
		hidraw_desc.size = desc_size;

		ret = common::ioctl(m_fd, HIDIOCGRDESC, &hidraw_desc);
		if (ret == -1)
			throw common::cerror("Failed to read HID descriptor");

		m_desc.load(gsl::span<u8> {&hidraw_desc.value[0], desc_size});
	}

	/*!
	 * The vendor ID of the HID device.
	 *
	 * @return The vendor ID of the HID device.
	 */
	[[nodiscard]] i16 product() const
	{
		return m_devinfo.product;
	}

	/*!
	 * The product ID of the HID device.
	 *
	 * @return The product ID of the HID device.
	 */
	[[nodiscard]] i16 vendor() const
	{
		return m_devinfo.vendor;
	}

	/*!
	 * The HID descriptor of the device.
	 *
	 * @return A reference to the wrapped HID descriptor of the device.
	 */
	[[nodiscard]] const Descriptor &descriptor() const
	{
		return m_desc;
	}

	/*!
	 * Reads a report from the HID device.
	 *
	 * @param[in] buffer The target storage for the report.
	 * @return The size of the report that was read in bytes.
	 */
	[[nodiscard]] isize read(gsl::span<u8> buffer) const
	{
		const isize ret = common::read(m_fd, buffer);
		if (ret == -1)
			throw common::cerror("Failed to read from HID device");

		return ret;
	}

	/*!
	 * Gets the data of a HID feature report.
	 *
	 * @param[in] report The report ID to get, followed by enough space to fit the data.
	 */
	void get_feature(gsl::span<u8> report) const
	{
		const int ret = common::ioctl(m_fd, HIDIOCGFEATURE(report.size()), report.data());
		if (ret == -1)
			throw common::cerror("Failed to get feature");
	}

	/*!
	 * Sets the data of a HID feature report.
	 *
	 * @param[in] report The report ID to set, followed by the new data.
	 */
	void set_feature(gsl::span<u8> report) const
	{
		const int ret = common::ioctl(m_fd, HIDIOCSFEATURE(report.size()), report.data());
		if (ret == -1)
			throw common::cerror("Failed to set feature");
	}
};

} // namespace iptsd::hid

#endif // IPTSD_HID_DEVICE_HPP
