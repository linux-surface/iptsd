// SPDX-License-Identifier: GPL-2.0-or-later

#include "device.hpp"

#include <cstddef>
#include <iterator>
#include <vector>

namespace iptsd::ipts {

std::size_t Device::buffer_size()
{
	std::size_t size = 0;
	auto &desc = this->descriptor();

	for (auto report : desc.reports(HIDRD_ITEM_MAIN_TAG_INPUT)) {
		auto usage = desc.usage(report);

		if (usage.size() != 2)
			continue;

		if (usage[0] != 0x56 || usage[1] != 0x61)
			continue;

		size = std::max(size, desc.size(report));
	}

	return size;
}

void Device::set_mode(bool multitouch)
{
	std::vector<u8> report;

	report.push_back(0x5);

	if (multitouch)
		report.push_back(0x1);
	else
		report.push_back(0x0);

	this->set_feature(report);
}

} // namespace iptsd::ipts
