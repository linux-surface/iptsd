// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_DESCRIPTOR_HPP
#define IPTSD_IPTS_DESCRIPTOR_HPP

#include "protocol.hpp"

#include <common/types.hpp>
#include <hid/report.hpp>
#include <hid/usage.hpp>

#include <algorithm>
#include <optional>
#include <vector>

namespace iptsd::ipts {

class Descriptor {
private:
	std::vector<hid::Report> m_reports {};

public:
	Descriptor(const std::vector<hid::Report> &reports) : m_reports {reports} {};

	/*!
	 * Tries to find all reports that contain touch data in the HID descriptor.
	 *
	 * @return A list of reports containing IPTS touch data.
	 */
	[[nodiscard]] std::vector<hid::Report> find_touch_data_reports() const
	{
		std::vector<hid::Report> out {};

		for (const hid::Report &report : m_reports) {
			if (is_touch_data_report(report))
				out.push_back(report);
		}

		return out;
	}

	/*!
	 * Tries to find the report for modesetting in the HID descriptor.
	 *
	 * @return The HID report for modesetting if it exists, null otherwise.
	 */
	[[nodiscard]] std::optional<hid::Report> find_modesetting_report() const
	{
		for (const hid::Report &report : m_reports) {
			if (is_modesetting_report(report))
				return report;
		}

		return std::nullopt;
	}

	/*!
	 * Tries to find the metadata report in the HID descriptor.
	 *
	 * @return The HID report for fetching metadata if it exists, null otherwise.
	 */
	[[nodiscard]] std::optional<hid::Report> find_metadata_report() const
	{
		for (const hid::Report &report : m_reports) {
			if (is_metadata_report(report))
				return report;
		}

		return std::nullopt;
	}

	/*!
	 * Checks whether a HID report matches the properties for an IPTS touch data report.
	 *
	 * @param[in] report The HID report to check.
	 * @return Whether the given report contains touch data.
	 */
	static bool is_touch_data_report(const hid::Report &report)
	{
		const std::unordered_set<hid::Usage> &usages = report.usages();

		if (report.type() != hid::ReportType::Input)
			return false;

		if (usages.size() != 2)
			return false;

		return report.find_usage(IPTS_HID_REPORT_USAGE_PAGE_DIGITIZER,
					 IPTS_HID_REPORT_USAGE_SCAN_TIME) &&
		       report.find_usage(IPTS_HID_REPORT_USAGE_PAGE_DIGITIZER,
					 IPTS_HID_REPORT_USAGE_GESTURE_DATA);
	}

	/*!
	 * Checks whether a HID report matches the properties for the modesetting report.
	 *
	 * @param[in] report The HID report to check.
	 * @return Whether the given report is a modesetting report.
	 */
	static bool is_modesetting_report(const hid::Report &report)
	{
		const std::unordered_set<hid::Usage> &usages = report.usages();

		if (report.type() != hid::ReportType::Feature)
			return false;

		if (usages.size() != 1)
			return false;

		if (report.size() != 8)
			return false;

		return report.find_usage(IPTS_HID_REPORT_USAGE_PAGE_VENDOR,
					 IPTS_HID_REPORT_USAGE_SET_MODE);
	}

	/*!
	 * Checks whether a HID report matches the properties of the metadata report.
	 *
	 * @param[in] report The HID report to check.
	 * @return Whether the given report is a metadata report.
	 */
	static bool is_metadata_report(const hid::Report &report)
	{
		const std::unordered_set<hid::Usage> &usages = report.usages();

		if (report.type() != hid::ReportType::Feature)
			return false;

		if (usages.size() != 1)
			return false;

		return report.find_usage(IPTS_HID_REPORT_USAGE_PAGE_DIGITIZER,
					 IPTS_HID_REPORT_USAGE_METADATA);
	}
};

} // namespace iptsd::ipts

#endif // IPTSD_IPTS_DESCRIPTOR_HPP
