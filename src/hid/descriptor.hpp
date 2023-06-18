// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_DESCRIPTOR_HPP
#define IPTSD_HID_DESCRIPTOR_HPP

#include "report.hpp"
#include "spec.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>

#include <gsl/gsl>

#include <optional>
#include <set>
#include <vector>

namespace iptsd::hid {

class Descriptor {
private:
	std::vector<Report> m_reports {};

public:
	Descriptor() = default;
	Descriptor(std::vector<Report> reports) : m_reports {std::move(reports)} {};

	/*!
	 * Returns all reports of a certain type.
	 *
	 * @param[in] type The type of report to search for.
	 * @return A list of all report IDs of the given type.
	 */
	[[nodiscard]] std::set<std::optional<u8>> reports(const ReportType type) const
	{
		std::set<std::optional<u8>> reports {};

		for (const Report &report : m_reports) {
			if (report.type() != type)
				continue;

			reports.insert(report.id());
		}

		return reports;
	}

	/*!
	 * Returns the usage tags of a HID report.
	 *
	 * @param[in] report The report ID for which to get the usage tags.
	 * @return All usage tags that apply to the report.
	 */
	[[nodiscard]] std::vector<Usage> usage(const std::optional<u8> id) const
	{
		std::vector<Usage> usages {};

		for (const Report &report : m_reports) {
			if (report.id() != id)
				continue;

			for (const Usage &usage : report.usages())
				usages.push_back(usage);
		}

		return usages;
	}

	/*!
	 * Calculates the size of a HID report.
	 *
	 * @param[in] report The report ID for which to calculate the size.
	 * @return The combined size of the HID report in bytes.
	 */
	[[nodiscard]] usize size(const std::optional<u8> id) const
	{
		f64 total_size = 0;

		for (const Report &report : m_reports) {
			if (report.id() != id)
				continue;

			total_size += casts::to<f64>(report.size()) / 8.0;
		}

		return casts::to<usize>(std::ceil(total_size));
	}
};

} // namespace iptsd::hid

#endif // IPTSD_HID_DESCRIPTOR_HPP
