// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_GENERIC_DEVICE_HPP
#define IPTSD_CORE_GENERIC_DEVICE_HPP

#include <common/types.hpp>
#include <ipts/device.hpp>
#include <ipts/metadata.hpp>

#include <optional>

namespace iptsd::core {

/*
 * Contains informations about the device that produced the data
 * that is processed by an application.
 */
struct DeviceInfo {
	u16 vendor = 0;
	u16 product = 0;
	ipts::Device::Type type {};
	std::optional<ipts::Metadata> meta = std::nullopt;

	[[nodiscard]] bool is_touchscreen() const
	{
		return this->type == ipts::Device::Type::Touchscreen;
	}

	[[nodiscard]] bool is_touchpad() const
	{
		return this->type == ipts::Device::Type::Touchpad;
	}
};

} // namespace iptsd::core

#endif // IPTSD_CORE_GENERIC_DEVICE_HPP
