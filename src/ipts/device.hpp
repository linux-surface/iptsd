// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_DEVICE_HPP
#define IPTSD_IPTS_DEVICE_HPP

#include "parser.hpp"

#include <common/types.hpp>
#include <hid/device.hpp>

#include <gsl/gsl>
#include <optional>
#include <vector>

namespace iptsd::ipts {

class Device : public hid::Device {
public:
	Device(const std::string &path) : hid::Device(path) {};

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
	 * Reads the IPTS device metadata from the metadata feature report.
	 *
	 * @return The metadata of the current device, or null if the report is not supported.
	 */
	[[nodiscard]] std::optional<const Metadata> get_metadata() const
	{
		std::optional<Metadata> metadata = std::nullopt;
		const hid::Descriptor &desc = this->descriptor();

		const u8 id = this->get_metadata_report_id();
		if (!id)
			return std::nullopt;

		std::vector<u8> report(desc.size(id) + 1);
		report.at(0) = id;

		this->get_feature(report);

		Parser parser;
		parser.on_metadata = [&](const Metadata &m) { metadata = m; };
		parser.parse<u8>(report);

		return metadata;
	}

private:
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

} // namespace iptsd::ipts

#endif // IPTSD_IPTS_DEVICE_HPP
