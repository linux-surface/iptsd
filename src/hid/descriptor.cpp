// SPDX-License-Identifier: GPL-2.0-or-later

#include "descriptor.hpp"

#include <common/types.hpp>
#include <hid/shim/hidrd.h>

#include <algorithm>
#include <cstddef>
#include <gsl/gsl>
#include <vector>

namespace iptsd::hid {

void Descriptor::load(gsl::span<u8> raw)
{
	std::size_t size = 0;

	this->descriptor.clear();
	this->descriptor.resize(raw.size());

	std::copy(raw.begin(), raw.end(), this->descriptor.begin());

	while (size < this->descriptor.size()) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto item = reinterpret_cast<const hidrd_item *>(&this->descriptor[size]);
		size += hidrd_item_get_size(item);

		this->parsed.push_back(item);
	}
}

const std::vector<const hidrd_item *> &Descriptor::items() const
{
	return this->parsed;
}

static bool is_report(const hidrd_item *item)
{
	if (hidrd_item_basic_get_format(item) != HIDRD_ITEM_BASIC_FORMAT_SHORT)
		return false;

	if (hidrd_item_short_get_type(item) != HIDRD_ITEM_SHORT_TYPE_GLOBAL)
		return false;

	return hidrd_item_global_get_tag(item) == HIDRD_ITEM_GLOBAL_TAG_REPORT_ID;
}

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

std::vector<u8> Descriptor::reports(hidrd_item_main_tag type) const
{
	u8 current = 0;
	std::vector<u8> reports {};

	for (auto item : this->items()) {
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

std::vector<hidrd_usage> Descriptor::usage(u8 report) const
{
	u8 current = 0;
	std::vector<hidrd_usage> usage {};

	for (auto item : this->items()) {
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

hidrd_usage_page Descriptor::usage_page(u8 report) const
{
	hidrd_usage_page current = HIDRD_USAGE_PAGE_MAX;

	for (auto item : this->items()) {
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

u64 Descriptor::size(u8 report) const
{
	u8 current = 0;

	u32 report_count = 0;
	u32 report_size = 0;
	u64 total_size = 0;

	for (auto item : this->items()) {
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

	return total_size;
}

} // namespace iptsd::hid
