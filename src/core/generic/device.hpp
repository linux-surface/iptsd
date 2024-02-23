// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_GENERIC_DEVICE_HPP
#define IPTSD_CORE_GENERIC_DEVICE_HPP

#include <common/types.hpp>
#include <ipts/device.hpp>

namespace iptsd::core {

/*
 * Contains informations about the device that produced the data
 * that is processed by an application.
 */
struct DeviceInfo {
	u16 vendor;
	u16 product;
	ipts::Device::Type type;
};

} // namespace iptsd::core

#endif // IPTSD_CORE_GENERIC_DEVICE_HPP
