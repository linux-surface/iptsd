// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_GENERIC_DEVICE_HPP
#define IPTSD_CORE_GENERIC_DEVICE_HPP

#include <common/types.hpp>

namespace iptsd::core {

/*
 * Contains informations about the device that produced the data
 * that is processed by an application.
 *
 * This struct is packed so that it can be part of binary dumps.
 * It is padded manually to keep compatibility with older files.
 */
struct [[gnu::packed]] DeviceInfo {
	u16 vendor;
	u16 product;
	u8 padding[4]; // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
	u64 buffer_size;
};

static_assert(sizeof(DeviceInfo) == 16);

} // namespace iptsd::core

#endif // IPTSD_CORE_GENERIC_DEVICE_HPP
