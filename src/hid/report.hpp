// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_REPORT_HPP
#define IPTSD_HID_REPORT_HPP

#include "spec.hpp"
#include "usage.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>

#include <optional>
#include <vector>

namespace iptsd::hid {

/*
 * The type of a report.
 */
enum class ReportType {
	// Data that is coming from the device
	Input,

	// Data that is sent to the device
	Output,

	// Data that can be queried and modified
	Feature,
};

/*
 * Describes the type, size and purpose of a HID report.
 */
class Report {
private:
	// The type of the report
	ReportType m_type;

	// The ID of the HID report. Input reports can have no ID.
	std::optional<u8> m_report_id;

	// The size of the report
	u32 m_report_size;
	u32 m_report_count;

	// The usage values describing the report
	std::vector<Usage> m_usages;

public:
	Report(ReportType type,
	       std::optional<u8> report_id,
	       u32 report_count,
	       u32 report_size,
	       const std::vector<Usage> &usages)
		: m_type {type}
		, m_report_id {report_id}
		, m_report_size {report_size}
		, m_report_count {report_count}
		, m_usages {usages} {};

	/*!
	 * The type of the HID report.
	 */
	[[nodiscard]] ReportType type() const
	{
		return m_type;
	}

	/*!
	 * The ID of the HID report.
	 */
	[[nodiscard]] std::optional<u8> id() const
	{
		return m_report_id;
	}

	/*!
	 * The total size of the HID report in bits.
	 */
	[[nodiscard]] u64 size() const
	{
		return casts::to<u64>(m_report_size) * m_report_count;
	}

	/*
	 * The usage tags of the HID report.
	 */
	[[nodiscard]] const std::vector<Usage> &usages() const
	{
		return m_usages;
	}
};

} // namespace iptsd::hid

#endif // IPTSD_HID_REPORT_HPP
