// SPDX-License-Identifier: GPL-2.0-or-later

#include "device.hpp"

#include "protocol.hpp"

#include <cstddef>
#include <iterator>
#include <vector>

namespace iptsd::ipts {

bool Device::is_touch_data(u8 report)
{
	auto &desc = this->descriptor();
	auto usage = desc.usage(report);

	if (usage.size() != 2)
		return false;

	if (usage[0] != IPTS_HID_REPORT_USAGE_SCAN_TIME)
		return false;

	if (usage[1] != IPTS_HID_REPORT_USAGE_GESTURE_DATA)
		return false;

	return desc.usage_page(report) == HIDRD_USAGE_PAGE_DIGITIZER;
}

bool Device::is_set_mode(u8 report)
{
	auto &desc = this->descriptor();
	auto usage = desc.usage(report);

	if (usage.size() != 1)
		return false;

	if (usage[0] != IPTS_HID_REPORT_USAGE_SET_MODE)
		return false;

	if (desc.size(report) != 1)
		return false;

	return desc.usage_page(report) == HIDRD_USAGE_PAGE_DIGITIZER;
}

u8 Device::get_set_mode()
{
	auto &desc = this->descriptor();

	for (auto report : desc.reports(HIDRD_ITEM_MAIN_TAG_FEATURE)) {
		if (this->is_set_mode(report))
			return report;
	}

	return 0;
}

bool Device::is_metadata_report(u8 report)
{
	auto &desc = this->descriptor();
	auto usage = desc.usage(report);

	if (usage.size() != 1)
		return false;

	if (usage[0] != IPTS_HID_REPORT_USAGE_METADATA)
		return false;

	return desc.usage_page(report) == HIDRD_USAGE_PAGE_DIGITIZER;
}

u8 Device::get_metadata_report_id()
{
	auto &desc = this->descriptor();

	for (auto report : desc.reports(HIDRD_ITEM_MAIN_TAG_FEATURE)) {
		if (this->is_metadata_report(report))
			return report;
	}

	return 0;
}

std::size_t Device::buffer_size()
{
	std::size_t size = 0;
	auto &desc = this->descriptor();

	for (auto report : desc.reports(HIDRD_ITEM_MAIN_TAG_INPUT))
		size = std::max(size, desc.size(report));

	return size;
}

void Device::set_mode(bool multitouch)
{
	std::vector<u8> report;

	report.push_back(this->get_set_mode());

	if (multitouch)
		report.push_back(0x1);
	else
		report.push_back(0x0);

	this->set_feature(report);
}

std::optional<const Metadata> Device::get_metadata()
{
	std::optional<Metadata> metadata = std::nullopt;
	auto &desc = this->descriptor();

	u8 id = this->get_metadata_report_id();
	if (!id)
		return std::nullopt;

	std::vector<u8> report(desc.size(id) + 1);
	report.at(0) = id;

	this->get_feature(report);

	Parser parser;
	parser.on_metadata = [&](const auto &m) { metadata = m; };
	parser.parse<u8>(report);

	return metadata;
}

} // namespace iptsd::ipts
