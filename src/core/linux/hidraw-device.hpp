// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_HIDRAW_DEVICE_HPP
#define IPTSD_CORE_LINUX_HIDRAW_DEVICE_HPP

#include "syscalls.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>
#include <core/generic/device.hpp>
#include <hid/descriptor.hpp>
#include <hid/parser.hpp>
#include <hid/report.hpp>
#include <hid/spec.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.hpp>

#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include <linux/hidraw.h>

#include <algorithm>
#include <filesystem>
#include <optional>
#include <set>
#include <vector>

namespace iptsd::core::linux {

class HidrawDevice {
private:
	int m_fd = -1;
	struct hidraw_devinfo m_devinfo {};

	// The HID descriptor of the device.
	hid::Descriptor m_desc {};

public:
	HidrawDevice(const std::filesystem::path &path) : m_fd {syscalls::open(path, O_RDWR)}
	{
		u32 desc_size = 0;

		syscalls::ioctl(m_fd, HIDIOCGRAWINFO, &m_devinfo);
		syscalls::ioctl(m_fd, HIDIOCGRDESCSIZE, &desc_size);

		struct hidraw_report_descriptor hidraw_desc {};
		hidraw_desc.size = desc_size;

		syscalls::ioctl(m_fd, HIDIOCGRDESC, &hidraw_desc);

		m_desc = hid::parse(gsl::span<u8> {&hidraw_desc.value[0], desc_size});
	}

	/*!
	 * Returns information about the device (vendor, product and buffer size).
	 *
	 * @return An object describing the device.
	 */
	[[nodiscard]] DeviceInfo info() const
	{
		DeviceInfo info {};

		/*
		 * The value is just an ID stored in a signed value.
		 * A negative device ID doesn't make sense, so cast it away.
		 */
		info.vendor = gsl::narrow_cast<u16>(m_devinfo.vendor);
		info.product = gsl::narrow_cast<u16>(m_devinfo.product);

		info.buffer_size = casts::to<u64>(this->buffer_size());

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
		return syscalls::read(m_fd, buffer);
	}

	/*!
	 * Changes the mode of the IPTS device.
	 *
	 * If the device is already in the requested mode, nothing will happen.
	 *
	 * @param[in] multitouch Whether multitouch mode should be enabled.
	 */
	void set_mode(const bool multitouch) const
	{
		std::array<u8, 2> report {
			this->get_set_mode().value_or(0),
			multitouch ? casts::to<u8>(0x1) : casts::to<u8>(0x0),
		};

		this->set_feature(report);
	}

	/*!
	 * Checks whether a HID report matches the properties for an IPTS touch data report.
	 *
	 * @param[in] report The ID of the HID report to check.
	 * @return Whether the given report contains touchscreen data.
	 */
	[[nodiscard]] bool is_touch_data(const std::optional<u8> report) const
	{
		const hid::Descriptor &desc = this->descriptor();
		const std::vector<hid::Usage> usage = desc.usage(report);

		if (usage.size() != 2)
			return false;

		return usage[0].page == IPTS_HID_REPORT_USAGE_PAGE_DIGITIZER &&
		       usage[0].value == IPTS_HID_REPORT_USAGE_SCAN_TIME &&
		       usage[1].page == IPTS_HID_REPORT_USAGE_PAGE_DIGITIZER &&
		       usage[1].value == IPTS_HID_REPORT_USAGE_GESTURE_DATA;
	}

	/*!
	 * Checks if a feature report is responsible for modesetting.
	 *
	 * @return Whether the device has a modesetting report.
	 */
	[[nodiscard]] bool has_set_mode() const
	{
		const hid::Descriptor &desc = this->descriptor();
		const std::set<std::optional<u8>> reports = desc.reports(hid::ReportType::Feature);

		return std::any_of(
			reports.cbegin(), reports.cend(),
			[&](const std::optional<u8> report) { return this->is_set_mode(report); });
	}

	/*!
	 * Checks if an input report contains touch data.
	 *
	 * @return Whether the device has a report containing touch data.
	 */
	[[nodiscard]] bool has_touch_data() const
	{
		const hid::Descriptor &desc = this->descriptor();
		const std::set<std::optional<u8>> reports = desc.reports(hid::ReportType::Input);

		return std::any_of(reports.cbegin(), reports.cend(),
				   [&](const std::optional<u8> report) {
					   return this->is_touch_data(report);
				   });
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

		const std::optional<u8> id = this->get_metadata_report_id();
		if (!id.has_value())
			return std::nullopt;

		std::vector<u8> report(desc.size(id) + 1);
		report.at(0) = id.value();

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

		for (const std::optional<u8> report : desc.reports(hid::ReportType::Input))
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
		syscalls::ioctl(m_fd, HIDIOCGFEATURE(report.size()), report.data());
	}

	/*!
	 * Sets the data of a HID feature report.
	 *
	 * @param[in] report The report ID to set, followed by the new data.
	 */
	void set_feature(const gsl::span<u8> report) const
	{
		syscalls::ioctl(m_fd, HIDIOCSFEATURE(report.size()), report.data());
	}

	/*!
	 * Checks whether a HID report matches the properties for the modesetting report.
	 *
	 * @param[in] report The ID of the HID report to check.
	 * @return Whether the given report is a modesetting report.
	 */
	[[nodiscard]] bool is_set_mode(const std::optional<u8> report) const
	{
		const hid::Descriptor &desc = this->descriptor();
		const std::vector<hid::Usage> usage = desc.usage(report);

		if (usage.size() != 1)
			return false;

		if (desc.size(report) != 1)
			return false;

		return usage[0].page == IPTS_HID_REPORT_USAGE_PAGE_VENDOR &&
		       usage[0].value == IPTS_HID_REPORT_USAGE_SET_MODE;
	}

	/*!
	 * Tries to find the report for modesetting in the HID descriptor.
	 *
	 * @return The report ID of the report for modesetting if it exists, 0 otherwise.
	 */
	[[nodiscard]] std::optional<u8> get_set_mode() const
	{
		const hid::Descriptor &desc = this->descriptor();

		for (const std::optional<u8> report : desc.reports(hid::ReportType::Feature)) {
			if (this->is_set_mode(report))
				return report;
		}

		return std::nullopt;
	}

	/*!
	 * Checks whether a HID report matches the properties of the metadata report.
	 *
	 * @param[in] report The ID of the HID report to check.
	 * @return Whether the given report is a metadata report.
	 */
	[[nodiscard]] bool is_metadata_report(const std::optional<u8> report) const
	{
		const hid::Descriptor &desc = this->descriptor();
		const std::vector<hid::Usage> usage = desc.usage(report);

		if (usage.size() != 1)
			return false;

		return usage[0].page == IPTS_HID_REPORT_USAGE_PAGE_DIGITIZER &&
		       usage[0].value == IPTS_HID_REPORT_USAGE_METADATA;
	}

	/*!
	 * Tries to find the metadata report in the HID descriptor.
	 *
	 * @return The report ID of the metadata report if it exists, 0 otherwise.
	 */
	[[nodiscard]] std::optional<u8> get_metadata_report_id() const
	{
		const hid::Descriptor &desc = this->descriptor();

		for (const std::optional<u8> report : desc.reports(hid::ReportType::Feature)) {
			if (this->is_metadata_report(report))
				return report;
		}

		return std::nullopt;
	}
};

} // namespace iptsd::core::linux

#endif // IPTSD_CORE_LINUX_HIDRAW_DEVICE_HPP
