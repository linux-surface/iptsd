// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_DESCRIPTOR_HPP
#define IPTSD_IPTS_DESCRIPTOR_HPP

#include "protocol/descriptor.hpp"

#include <common/types.hpp>
#include <hid/report.hpp>
#include <hid/usage.hpp>

#include <optional>
#include <vector>

namespace iptsd::ipts {

class Descriptor {
private:
	std::vector<hid::Report> m_reports {};

public:
	Descriptor(const std::vector<hid::Report> &reports) : m_reports {reports} {};

	/*!
	 * Searches for all reports that contain touch data in the HID descriptor.
	 *
	 * @return A list of reports containing IPTS touch data.
	 */
	[[nodiscard]] std::vector<hid::Report> find_touch_data_reports() const
	{
		std::vector<hid::Report> out {};

		for (const hid::Report &report : m_reports) {
			if (protocol::descriptor::is_touch_data(report))
				out.push_back(report);
		}

		return out;
	}

	/*!
	 * Searches for the modesetting report in the HID descriptor.
	 *
	 * The modesetting report is a feature report for changing the mode of the device from
	 * singletouch to multitouch and vice versa.
	 *
	 * @return The HID report for modesetting if it exists, null otherwise.
	 */
	[[nodiscard]] std::optional<hid::Report> find_modesetting_report() const
	{
		for (const hid::Report &report : m_reports) {
			if (protocol::descriptor::is_set_mode(report))
				return report;
		}

		return std::nullopt;
	}

	/*!
	 * Searches for the metadata report in the HID descriptor.
	 *
	 * The metadata report is a feature report that returns data about the device, such as
	 * the screen size or the orientation of the touch data.
	 *
	 * @return The HID report for fetching metadata if it exists, null otherwise.
	 */
	[[nodiscard]] std::optional<hid::Report> find_metadata_report() const
	{
		for (const hid::Report &report : m_reports) {
			if (protocol::descriptor::is_metadata(report))
				return report;
		}

		return std::nullopt;
	}
};

} // namespace iptsd::ipts

#endif // IPTSD_IPTS_DESCRIPTOR_HPP
