// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_HIDRAW_DEVICE_HPP
#define IPTSD_CORE_LINUX_HIDRAW_DEVICE_HPP

#include <common/cerror.hpp>
#include <common/cwrap.hpp>
#include <common/types.hpp>
#include <core/generic/device.hpp>
#include <hid/descriptor.hpp>
#include <ipts/parser.hpp>

#include <filesystem>
#include <gsl/gsl>
#include <gsl/util>
#include <linux/hidraw.h>
#include <optional>
#include <vector>

namespace iptsd::core::linux {

class HidrawDevice {
private:
	int m_fd = -1;
	struct hidraw_devinfo m_devinfo {};

	// The HID descriptor of the device.
	hid::Descriptor m_desc {};

public:
	HidrawDevice(const std::filesystem::path &path)
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
	 * Returns information about the device (vendor, product and buffer size).
	 *
	 * @return An object describing the device.
	 */
	[[nodiscard]] DeviceInfo info() const
	{
		DeviceInfo info {};

		info.vendor = gsl::narrow_cast<u16>(m_devinfo.vendor);
		info.product = gsl::narrow_cast<u16>(m_devinfo.product);
		info.buffer_size = gsl::narrow<u64>(this->buffer_size());

		return info;
	}

	/*!
	 * The HID descriptor of the device.
	 *
	 * @return A reference to the wrapped HID descriptor of the device.
	 */
	[[nodiscard]] const hid::Descriptor &descriptor() const
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
	 * Changes the mode of the IPTS device.
	 *
	 * If the device is already in the requested mode, nothing will happen.
	 *
	 * @param[in] multitouch Whether multitouch mode should be enabled.
	 */
	void set_mode(bool multitouch) const
	{
		std::array<u8, 2> report {
			this->get_set_mode(),
			multitouch ? static_cast<u8>(0x1) : static_cast<u8>(0x0),
		};

		this->set_feature(report);
	}

	/*!
	 * Checks whether a HID report matches the properties for an IPTS touch data report.
	 *
	 * @param[in] report The ID of the HID report to check.
	 * @return Whether the given report contains touchscreen data.
	 */
	[[nodiscard]] bool is_touch_data(u8 report) const
	{
		const hid::Descriptor &desc = this->descriptor();
		const std::vector<hidrd_usage> usage = desc.usage(report);

		if (usage.size() != 2)
			return false;

		if (usage[0] != IPTS_HID_REPORT_USAGE_SCAN_TIME)
			return false;

		if (usage[1] != IPTS_HID_REPORT_USAGE_GESTURE_DATA)
			return false;

		return desc.usage_page(report) == HIDRD_USAGE_PAGE_DIGITIZER;
	}

	/*!
	 * Reads the IPTS device metadata from the metadata feature report.
	 *
	 * @return The metadata of the current device, or null if the report is not supported.
	 */
	[[nodiscard]] std::optional<const ipts::Metadata> get_metadata() const
	{
		std::optional<ipts::Metadata> metadata = std::nullopt;
		const hid::Descriptor &desc = this->descriptor();

		const u8 id = this->get_metadata_report_id();
		if (!id)
			return std::nullopt;

		std::vector<u8> report(desc.size(id) + 1);
		report.at(0) = id;

		this->get_feature(report);

		ipts::Parser parser;
		parser.on_metadata = [&](const ipts::Metadata &m) { metadata = m; };
		parser.parse<u8>(report);

		return metadata;
	}

private:
	/*!
	 * Determines the required size for a buffer holding IPTS touch data.
	 *
	 * @return The size of the largest touch data report that IPTS can send.
	 */
	[[nodiscard]] usize buffer_size() const
	{
		usize size = 0;
		const hid::Descriptor &desc = this->descriptor();

		for (const u8 report : desc.reports(HIDRD_ITEM_MAIN_TAG_INPUT))
			size = std::max(size, desc.size(report));

		return size;
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

	/*!
	 * Checks whether a HID report matches the properties for the modesetting report.
	 *
	 * @param[in] report The ID of the HID report to check.
	 * @return Whether the given report is a modesetting report.
	 */
	[[nodiscard]] bool is_set_mode(u8 report) const
	{
		const hid::Descriptor &desc = this->descriptor();
		const std::vector<hidrd_usage> usage = desc.usage(report);

		if (usage.size() != 1)
			return false;

		if (usage[0] != IPTS_HID_REPORT_USAGE_SET_MODE)
			return false;

		if (desc.size(report) != 1)
			return false;

		return desc.usage_page(report) == HIDRD_USAGE_PAGE_DIGITIZER;
	}

	/*!
	 * Tries to find the report for modesetting in the HID descriptor.
	 *
	 * @return The report ID of the report for modesetting if it exists, 0 otherwise.
	 */
	[[nodiscard]] u8 get_set_mode() const
	{
		const hid::Descriptor &desc = this->descriptor();

		for (const u8 report : desc.reports(HIDRD_ITEM_MAIN_TAG_FEATURE)) {
			if (this->is_set_mode(report))
				return report;
		}

		return 0;
	}

	/*!
	 * Checks whether a HID report matches the properties of the metadata report.
	 *
	 * @param[in] report The ID of the HID report to check.
	 * @return Whether the given report is a metadata report.
	 */
	[[nodiscard]] bool is_metadata_report(u8 report) const
	{
		const hid::Descriptor &desc = this->descriptor();
		const std::vector<hidrd_usage> usage = desc.usage(report);

		if (usage.size() != 1)
			return false;

		if (usage[0] != IPTS_HID_REPORT_USAGE_METADATA)
			return false;

		return desc.usage_page(report) == HIDRD_USAGE_PAGE_DIGITIZER;
	}

	/*!
	 * Tries to find the metadata report in the HID descriptor.
	 *
	 * @return The report ID of the metadata report if it exists, 0 otherwise.
	 */
	[[nodiscard]] u8 get_metadata_report_id() const
	{
		const hid::Descriptor &desc = this->descriptor();

		for (const u8 report : desc.reports(HIDRD_ITEM_MAIN_TAG_FEATURE)) {
			if (this->is_metadata_report(report))
				return report;
		}

		return 0;
	}
};

} // namespace iptsd::core::linux

#endif // IPTSD_CORE_LINUX_HIDRAW_DEVICE_HPP
