// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_USAGE_HPP
#define IPTSD_HID_USAGE_HPP

#include <common/casts.hpp>
#include <common/types.hpp>

#include <functional>

namespace iptsd::hid {

struct Usage {
	u16 page;
	u16 value;

	bool operator==(const Usage &other) const
	{
		return this->page == other.page && this->value == other.value;
	}

	bool operator!=(const Usage &other) const
	{
		return !(*this == other);
	}
};

} // namespace iptsd::hid

/*
 * Implement std::hash for usage so it can be added to unordered_set.
 */
template <>
struct std::hash<iptsd::hid::Usage> {
public:
	usize operator()(const iptsd::hid::Usage &usage) const
	{
		using namespace iptsd;

		const u32 value = (casts::to<u32>(usage.page) << 16) + usage.value;
		return std::hash<u32> {}(value);
	}
};

#endif // IPTSD_HID_USAGE_HPP
