// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_GENERIC_DEVICE_HPP
#define IPTSD_CORE_GENERIC_DEVICE_HPP

#include <common/types.hpp>
#include <ipts/data.hpp>
#include <ipts/device.hpp>

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
};

} // namespace iptsd::core

#endif // IPTSD_CORE_GENERIC_DEVICE_HPP
