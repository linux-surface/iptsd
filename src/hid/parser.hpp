// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_PARSER_HPP
#define IPTSD_HID_PARSER_HPP

#include "report.hpp"
#include "spec.hpp"
#include "state.hpp"

#include <common/casts.hpp>
#include <common/reader.hpp>
#include <common/types.hpp>

#include <gsl/gsl>

#include <optional>
#include <vector>

namespace iptsd::hid {

/*!
 * Loads a HID descriptor from a its binary representation.
 *
 * This function will parse the report ID, size and usage tags.
 *
 * @param[in] buffer The buffer containing the descriptor.
 * @param[in] reports A reference to the vector where a representation of the reports will be saved.
 * @return A representation of the HID reports defined by the descriptor.
 */
inline void parse(const gsl::span<u8> buffer, std::vector<Report> &reports)
{
	ParserState state {};
	Reader reader {buffer};

	reports.clear();

	while (reader.size() > 0) {
		const u8 header = reader.read<u8>();

		const u8 size = (header & BITS_SIZE) >> SHIFT_SIZE;
		if (size == 0)
			continue;

		u32 data = 0;

		if (size == 1)
			data = casts::to<u32>(reader.read<u8>());
		else if (size == 2)
			data = casts::to<u32>(reader.read<u16>());
		else if (size == 3)
			data = reader.read<u32>();

		const auto type = gsl::narrow<ItemType>((header & BITS_TYPE) >> SHIFT_TYPE);

		if (type == ItemType::Main) {
			const auto tag = gsl::narrow<TagMain>((header & BITS_TAG) >> SHIFT_TAG);

			// We dont care about collections
			if (tag == TagMain::Collection || tag == TagMain::EndCollection) {
				state.reset_local();
				continue;
			}

			bool found = false;
			const Report nr = state.get_report(tag);

			// Try to find an existing report that we can update
			for (Report &report : reports) {
				if (nr.id() != report.id())
					continue;

				if (nr.type() != report.type())
					continue;

				report.merge(nr);

				found = true;
				break;
			}

			if (!found)
				reports.push_back(nr);
		} else if (type == ItemType::Global) {
			switch (gsl::narrow<TagGlobal>((header & BITS_TAG) >> SHIFT_TAG)) {
			case TagGlobal::ReportId:
				state.set_report_id(casts::to<u8>(data));
				break;
			case TagGlobal::ReportSize:
				state.set_report_size(data);
				break;
			case TagGlobal::ReportCount:
				state.set_report_count(data);
				break;
			case TagGlobal::UsagePage:
				state.set_usage_page(casts::to<u16>(data));
				break;
			}
		} else {
			switch (gsl::narrow<TagLocal>((header & BITS_TAG) >> SHIFT_TAG)) {
			case TagLocal::Usage:
				state.set_usage(casts::to<u16>(data));
				break;
			case TagLocal::UsageMaximum:
				state.set_usage_max(casts::to<u16>(data));
				break;
			case TagLocal::UsageMinimum:
				state.set_usage_min(casts::to<u16>(data));
				break;
			}
		}
	}
}

/*!
 * Loads a HID descriptor from a its binary representation.
 *
 * This function will parse the report ID, size and usage tags.
 *
 * @param[in] buffer The buffer containing the descriptor.
 * @return A representation of the HID reports defined by the descriptor.
 */
inline std::vector<Report> parse(const gsl::span<u8> buffer)
{
	std::vector<Report> reports {};
	parse(buffer, reports);
	return reports;
}

} // namespace iptsd::hid

#endif // IPTSD_HID_PARSER_HPP
