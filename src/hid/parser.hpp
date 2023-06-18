// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_PARSER_HPP
#define IPTSD_HID_PARSER_HPP

#include "descriptor.hpp"
#include "report.hpp"
#include "spec.hpp"
#include "state.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>
#include <ipts/reader.hpp>

#include <gsl/gsl>

#include <optional>
#include <vector>

namespace iptsd::hid {

/*!
 * Loads a HID descriptor from a its binary representation.
 *
 * This function will parse the report ID, size and usage tags.
 *
 * @param[in] data The buffer containing the descriptor.
 * @return A representation of the HID reports defined by the descriptor.
 */
inline Descriptor parse(const gsl::span<u8> buffer)
{
	ParserState state {};
	std::vector<Report> reports {};

	ipts::Reader reader {buffer};

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

			reports.push_back(state.get_report(tag));
		} else if (type == ItemType::Global) {
			const auto tag = gsl::narrow<TagGlobal>((header & BITS_TAG) >> SHIFT_TAG);

			switch (tag) {
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
			const auto tag = gsl::narrow<TagLocal>((header & BITS_TAG) >> SHIFT_TAG);

			switch (tag) {
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

	return Descriptor {std::move(reports)};
}

} // namespace iptsd::hid

#endif // IPTSD_HID_PARSER_HPP
