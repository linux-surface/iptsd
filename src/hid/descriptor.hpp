// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_DESCRIPTOR_HPP
#define IPTSD_HID_DESCRIPTOR_HPP

#include "shim/hidrd.h"

#include <common/casts.hpp>
#include <common/types.hpp>

#include <gsl/gsl>

#include <vector>

namespace iptsd::hid {

class Descriptor {
private:
	std::vector<u8> m_descriptor {};
	std::vector<const hidrd_item *> m_parsed {};

public:
	/*!
	 * Loads a binary HID descriptor and parses it.
	 *
	 * @param[in] raw The raw binary data to load and parse.
	 */
	void load(const gsl::span<u8> raw)
	{
		usize size = 0;

		m_descriptor.clear();
		m_descriptor.resize(raw.size());

		std::copy(raw.begin(), raw.end(), m_descriptor.begin());

		while (size < m_descriptor.size()) {
			// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
			const auto *item =
				reinterpret_cast<const hidrd_item *>(&m_descriptor[size]);
			// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

			size += hidrd_item_get_size(item);
			m_parsed.push_back(item);
		}
	}

	/*!
	 * All hidrd_items of the HID report.
	 *
	 * Every item corresponds to one tag of the HID descriptor.

	 * @return A list of all items found in the HID descriptor.
	 */
	[[nodiscard]] const std::vector<const hidrd_item *> &items() const
	{
		return m_parsed;
	}

	/*!
	 * Returns all reports matching a certain type.
	 *
	 * This can be used to get a list of all Input or Feature reports.
	 *
	 * @param[in] type The type of report to search for.
	 * @return A list of all report IDs of the given type.
	 */
	[[nodiscard]] std::vector<u8> reports(const hidrd_item_main_tag type) const
	{
		u8 current = 0;
		std::vector<u8> reports {};

		for (const hidrd_item *item : this->items()) {
			if (is_report(item)) {
				current = hidrd_item_report_id_get_value(item);
				continue;
			}

			if (!is_commit(item))
				continue;

			if (current == 0)
				continue;

			if (hidrd_item_main_get_tag(item) == type) {
				reports.push_back(current);
				current = 0;
			}
		}

		return reports;
	}

	/*!
	 * Returns the usage tags of a HID report.
	 *
	 * @param[in] report The report ID for which to get the usage tags.
	 * @return The values of all usage tags that are active for the report.
	 */
	[[nodiscard]] std::vector<hidrd_usage> usage(const u8 report) const
	{
		u8 current = 0;
		std::vector<hidrd_usage> usage {};

		for (const hidrd_item *item : this->items()) {
			if (is_report(item)) {
				current = hidrd_item_report_id_get_value(item);
				continue;
			}

			if (current != report)
				continue;

			if (hidrd_item_short_get_type(item) != HIDRD_ITEM_SHORT_TYPE_LOCAL)
				continue;

			if (hidrd_item_local_get_tag(item) != HIDRD_ITEM_LOCAL_TAG_USAGE)
				continue;

			usage.push_back(hidrd_item_usage_get_value(item));
		}

		return usage;
	}

	/*!
	 * The usage page of a HID report.
	 *
	 * @param[in] report The report ID for which to get the usage page.
	 * @return The value of the usage page tag that is active for the report.
	 */
	[[nodiscard]] hidrd_usage_page usage_page(const u8 report) const
	{
		hidrd_usage_page current = HIDRD_USAGE_PAGE_MAX;

		for (const hidrd_item *item : this->items()) {
			if (is_report(item) && report == hidrd_item_report_id_get_value(item))
				return current;

			if (hidrd_item_short_get_type(item) != HIDRD_ITEM_SHORT_TYPE_GLOBAL)
				continue;

			if (hidrd_item_global_get_tag(item) != HIDRD_ITEM_GLOBAL_TAG_USAGE_PAGE)
				continue;

			current = hidrd_item_usage_page_get_value(item);
		}

		return HIDRD_USAGE_PAGE_MAX;
	}

	/*!
	 * Calculates the size of a HID report.
	 *
	 * @param[in] report The report ID for which to calculate the size.
	 * @return The combined size of the HID report in bytes.
	 */
	[[nodiscard]] usize size(const u8 report) const
	{
		u8 current = 0;

		u32 report_count = 0;
		u32 report_size = 0;
		u64 total_size = 0;

		for (const hidrd_item *item : this->items()) {
			if (is_report(item)) {
				current = hidrd_item_report_id_get_value(item);
				continue;
			}

			if (current != report)
				continue;

			if (is_commit(item)) {
				total_size += (report_count * report_size) / 8;
				continue;
			}

			if (hidrd_item_short_get_type(item) != HIDRD_ITEM_SHORT_TYPE_GLOBAL)
				continue;

			if (hidrd_item_global_get_tag(item) == HIDRD_ITEM_GLOBAL_TAG_REPORT_COUNT) {
				report_count = hidrd_item_report_count_get_value(item);
				continue;
			}

			if (hidrd_item_global_get_tag(item) == HIDRD_ITEM_GLOBAL_TAG_REPORT_SIZE) {
				report_size = hidrd_item_report_size_get_value(item);
				continue;
			}
		}

		return casts::to<usize>(total_size);
	}

	/*!
	 * Whether an item represents a Report ID tag.
	 *
	 * @param[in] item The item to check.
	 * @return true if the item is the definition of a Report ID.
	 */
	static bool is_report(const hidrd_item *item)
	{
		if (hidrd_item_basic_get_format(item) != HIDRD_ITEM_BASIC_FORMAT_SHORT)
			return false;

		if (hidrd_item_short_get_type(item) != HIDRD_ITEM_SHORT_TYPE_GLOBAL)
			return false;

		return hidrd_item_global_get_tag(item) == HIDRD_ITEM_GLOBAL_TAG_REPORT_ID;
	}

	/*!
	 * Whether a hidrd item "commits" a report.
	 *
	 * With the state machine that HID uses, an Input, Output or Feature tag
	 * will create a new report with the current state, e.g. everything that preceded
	 * it.
	 *
	 * This checks whether an item represents one of these "committing" tags.
	 *
	 * @param[in] item The item to check.
	 * @return Whether the item is a "committing" tag.
	 */
	static bool is_commit(const hidrd_item *item)
	{
		if (hidrd_item_basic_get_format(item) != HIDRD_ITEM_BASIC_FORMAT_SHORT)
			return false;

		if (hidrd_item_short_get_type(item) != HIDRD_ITEM_SHORT_TYPE_MAIN)
			return false;

		return hidrd_item_main_get_tag(item) == HIDRD_ITEM_MAIN_TAG_INPUT ||
		       hidrd_item_main_get_tag(item) == HIDRD_ITEM_MAIN_TAG_OUTPUT ||
		       hidrd_item_main_get_tag(item) == HIDRD_ITEM_MAIN_TAG_FEATURE;
	}
};

} // namespace iptsd::hid

#endif // IPTSD_HID_DESCRIPTOR_HPP
