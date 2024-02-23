// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_DESCRIPTOR_HPP
#define IPTSD_IPTS_PROTOCOL_DESCRIPTOR_HPP

#include <common/types.hpp>
#include <hid/collection.hpp>
#include <hid/report.hpp>

namespace iptsd::ipts::protocol::descriptor {

// Needed because we have a hid namespace in iptsd::ipts::protocol
namespace hid = iptsd::hid;

constexpr u16 USAGE_PAGE_DIGITIZER = 0x000D;
constexpr u16 USAGE_PAGE_VENDOR = 0xFF00;

constexpr u8 USAGE_TOUCHSCREEN = 0x04;
constexpr u8 USAGE_TOUCHPAD = 0x05;

constexpr u8 USAGE_SCAN_TIME = 0x56;
constexpr u8 USAGE_GESTURE_DATA = 0x61;
constexpr u8 USAGE_SET_MODE = 0xC8;
constexpr u8 USAGE_METADATA = 0x63;

/*!
 * Checks if a given report contains touch data.
 *
 * @param[in] report The report to check.
 * @return Whether the report matches the properties for a touch data report.
 */
inline bool is_touch_data(const hid::Report &report)
{
	return report.type == hid::Report::Type::Input &&
	       report.has_usage(USAGE_PAGE_DIGITIZER, USAGE_SCAN_TIME) &&
	       report.has_usage(USAGE_PAGE_DIGITIZER, USAGE_GESTURE_DATA);
}

/*!
 * Checks if a given report sets the mode of the device.
 *
 * @param[in] report The report to check.
 * @return Whether the report matches the properties for a modesetting report.
 */
inline bool is_set_mode(const hid::Report &report)
{
	return report.type == hid::Report::Type::Feature && report.bytes() == 1 &&
	       report.has_usage(USAGE_PAGE_VENDOR, USAGE_SET_MODE);
}

/*!
 * Checks if a given report returns metadata for the device.
 *
 * @param[in] report The report to check.
 * @return Whether the report matches the properties for a metadata report.
 */
inline bool is_metadata(const hid::Report &report)
{
	return report.type == hid::Report::Type::Feature &&
	       report.has_usage(USAGE_PAGE_DIGITIZER, USAGE_METADATA);
}

/*!
 * Checks if a given collection indicates that this device is a touchscreen.
 *
 * @param[in] report The collection to check.
 * @return Whether the presence of the collection makes the device a touchscreen.
 */
inline bool is_touchscreen(const hid::Collection &collection)
{
	return collection.has_usage(USAGE_PAGE_DIGITIZER, USAGE_TOUCHSCREEN);
}

/*!
 * Checks if a given collection indicates that this device is a touchpad.
 *
 * @param[in] report The collection to check.
 * @return Whether the presence of the collection makes the device a touchpad.
 */
inline bool is_touchpad(const hid::Collection &collection)
{
	return collection.has_usage(USAGE_PAGE_DIGITIZER, USAGE_TOUCHPAD);
}

} // namespace iptsd::ipts::protocol::descriptor

#endif // IPTSD_IPTS_PROTOCOL_DESCRIPTOR_HPP
