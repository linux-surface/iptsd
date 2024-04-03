// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_ERRORS_HPP
#define IPTSD_HID_ERRORS_HPP

#include <common/types.hpp>

#include <string>

namespace iptsd::hid {

enum class Error : u8 {
	ReportMergeTypes,
	ReportMergeIDs,
	UsageBeforePage,
	InvalidReportType,
	MissingReportSize,
	MissingReportCount,
};

inline std::string format_as(Error err)
{
	switch (err) {
	case Error::ReportMergeTypes:
		return "hid: Cannot merge two reports with different types!";
	case Error::ReportMergeIDs:
		return "hid: Cannot merge two reports with different IDs!";
	case Error::UsageBeforePage:
		return "hid: Invalid descriptor, Usage before Usage Page!";
	case Error::InvalidReportType:
		return "hid: Invalid report type in HID descriptor!";
	case Error::MissingReportSize:
		return "hid: Missing report size in HID descriptor!";
	case Error::MissingReportCount:
		return "hid:: Missing report count in HID descriptor!";
	default:
		return "hid: Invalid error code!";
	}
}

} // namespace iptsd::hid

#endif // IPTSD_HID_ERRORS_HPP
