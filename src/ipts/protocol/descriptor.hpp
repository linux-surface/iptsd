// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_PROTOCOL_DESCRIPTOR_HPP
#define IPTSD_IPTS_PROTOCOL_DESCRIPTOR_HPP

#include <common/types.hpp>
#include <hid/report.hpp>

namespace iptsd::ipts::protocol::descriptor {

// Needed because we have a hid namespace in iptsd::ipts::protocol
namespace hid = iptsd::hid;

constexpr u16 USAGE_PAGE_DIGITIZER = 0x000D;
constexpr u16 USAGE_PAGE_VENDOR = 0xFF00;

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
	return report.type() == hid::ReportType::Input &&
	       report.find_usage(USAGE_PAGE_DIGITIZER, USAGE_SCAN_TIME) &&
	       report.find_usage(USAGE_PAGE_DIGITIZER, USAGE_GESTURE_DATA);
}

/*!
 * Checks if a given report sets the mode of the device.
 *
 * @param[in] report The report to check.
 * @return Whether the report matches the properties for a modesetting report.
 */
inline bool is_set_mode(const hid::Report &report)
{
	return report.type() == hid::ReportType::Feature && report.size() == 8 &&
	       report.find_usage(USAGE_PAGE_VENDOR, USAGE_SET_MODE);
}

/*!
 * Checks if a given report returns metadata for the device.
 *
 * @param[in] report The report to check.
 * @return Whether the report matches the properties for a metadata report.
 */
inline bool is_metadata(const hid::Report &report)
{
	return report.type() == hid::ReportType::Feature &&
	       report.find_usage(USAGE_PAGE_DIGITIZER, USAGE_METADATA);
}

} // namespace iptsd::ipts::protocol::descriptor

#endif // IPTSD_IPTS_PROTOCOL_DESCRIPTOR_HPP
