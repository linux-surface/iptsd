// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_REPORT_HPP
#define IPTSD_HID_REPORT_HPP

#include "errors.hpp"
#include "usage.hpp"

#include <common/casts.hpp>
#include <common/error.hpp>
#include <common/types.hpp>

#include <algorithm>
#include <optional>
#include <unordered_set>

namespace iptsd::hid {

/*
 * The type of a report.
 */
enum class ReportType : u8 {
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
	u64 m_report_size;

	// The usage values describing the report
	std::unordered_set<Usage> m_usages;

public:
	Report(const ReportType type,
	       const std::optional<u8> report_id,
	       const u32 report_count,
	       const u32 report_size,
	       const std::unordered_set<Usage> &usages)
		: m_type {type},
		  m_report_id {report_id},
		  m_report_size {casts::to<u64>(report_count) * report_size},
		  m_usages {usages} {};

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
		return m_report_size;
	}

	/*!
	 * The usage tags of the HID report.
	 */
	[[nodiscard]] const std::unordered_set<Usage> &usages() const
	{
		return m_usages;
	}

	/*!
	 * Checks if the report contains a certain usage tag.
	 *
	 * @param[in] value The usage tag to search for.
	 * @return Whether the combination of Usage / Usage Page applies to this report.
	 */
	[[nodiscard]] bool find_usage(const Usage value) const
	{
		return this->find_usage(value.page, value.value);
	}

	/*!
	 * Checks if the report contains a certain usage tag.
	 *
	 * @param[in] page The usage page tag to search for.
	 * @param[in] value The usage tag to search for.
	 * @return Whether the combination of Usage / Usage Page applies to this report.
	 */
	[[nodiscard]] bool find_usage(const u16 page, const u16 value) const
	{
		return std::any_of(m_usages.cbegin(),
		                   m_usages.cend(),
		                   [&](const Usage &usage) -> bool {
					   return usage.page == page && usage.value == value;
				   });
	}

	/*!
	 * Combines two reports sharing the same ID and type.
	 *
	 * @param[in] other The report to combine with this one.
	 */
	void merge(const Report &other)
	{
		if (m_type != other.type())
			throw common::Error<Error::ReportMergeTypes> {};

		if (m_report_id != other.id())
			throw common::Error<Error::ReportMergeIDs> {};

		m_report_size += other.size();

		for (const Usage &usage : other.usages())
			m_usages.insert(usage);
	}
};

} // namespace iptsd::hid

#endif // IPTSD_HID_REPORT_HPP
