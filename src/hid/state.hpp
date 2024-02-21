// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_STATE_HPP
#define IPTSD_HID_STATE_HPP

#include "errors.hpp"
#include "report.hpp"
#include "spec.hpp"
#include "usage.hpp"

#include <common/error.hpp>
#include <common/types.hpp>

#include <optional>
#include <unordered_set>

namespace iptsd::hid {

class ParserState {
private:
	std::optional<u8> m_report_id = std::nullopt;
	std::optional<u32> m_report_size = std::nullopt;
	std::optional<u32> m_report_count = std::nullopt;

	std::optional<u16> m_usage = std::nullopt;
	std::optional<u16> m_usage_page = std::nullopt;
	std::optional<u16> m_usage_min = std::nullopt;
	std::optional<u16> m_usage_max = std::nullopt;

	std::unordered_set<Usage> m_usages {};

public:
	/*!
	 * Resets all local options to their default values.
	 */
	void reset_local()
	{
		m_usages.clear();
		m_usage.reset();
		m_usage_min.reset();
		m_usage_max.reset();
	}

	/*!
	 * Resets all global options to their default values.
	 */
	void reset_global()
	{
		m_report_id.reset();
		m_report_count.reset();
		m_report_size.reset();
		m_usage_page.reset();
	}

	/*!
	 * Reset all options to their default values.
	 */
	void reset()
	{
		this->reset_local();
		this->reset_global();
	}

	/*!
	 * Update the stored report ID.
	 *
	 * @param[in] id The HID report ID.
	 */
	void set_report_id(const u8 id)
	{
		m_report_id = id;
	}

	/*!
	 * Update the stored report size.
	 *
	 * @param[in] size How large the current field is, in bits.
	 */
	void set_report_size(const u32 size)
	{
		m_report_size = size;
	}

	/*!
	 * Update the stored report count.
	 *
	 * @param[in] count How many instances of the current field there are.
	 */
	void set_report_count(const u32 count)
	{
		m_report_count = count;
	}

	/*!
	 * Update the stored usage value.
	 *
	 * This must be called after @ref set_usage_page.
	 *
	 * @param[in] usage The kind of data that is stored in the current field.
	 */
	void set_usage(const u16 usage)
	{
		if (!m_usage_page.has_value())
			throw common::Error<Error::UsageBeforePage> {};

		m_usages.insert(Usage {m_usage_page.value(), usage});
	}

	/*!
	 * Update the stored usage page value.
	 *
	 * @param[in] usage_page The category of data that is contained in the current field.
	 */
	void set_usage_page(const u16 usage_page)
	{
		m_usage_page = usage_page;
	}

	/*!
	 * Update the stored minimum usage.
	 *
	 * @param[in] usage_min The minimum value for Usage that applies to this field.
	 */
	void set_usage_min(const u16 usage_min)
	{
		if (!m_usage_page.has_value())
			throw common::Error<Error::UsageBeforePage> {};

		if (m_usage_max.has_value()) {
			for (u16 i = usage_min; i < (m_usage_max.value() + 1); i++)
				m_usages.insert(Usage {m_usage_page.value(), i});

			m_usage_max.reset();
		} else {
			m_usage_min = usage_min;
		}
	}

	/*!
	 * Update the stored maximum usage.
	 *
	 * @param[in] usage_max The maximum value for Usage that applies to this field.
	 */
	void set_usage_max(const u16 usage_max)
	{
		if (!m_usage_page.has_value())
			throw common::Error<Error::UsageBeforePage> {};

		if (m_usage_min.has_value()) {
			for (u16 i = m_usage_min.value(); i < (usage_max + 1); i++)
				m_usages.insert(Usage {m_usage_page.value(), i});

			m_usage_min.reset();
		} else {
			m_usage_max = usage_max;
		}
	}

	/*!
	 * Assembles a HID report from the current state.
	 *
	 * @param[in] tag The type of the report.
	 */
	[[nodiscard]] Report get_report(const TagMain tag)
	{
		ReportType type {};

		switch (tag) {
		case TagMain::Input:
			type = ReportType::Input;
			break;
		case TagMain::Output:
			type = ReportType::Output;
			break;
		case TagMain::Feature:
			type = ReportType::Feature;
			break;
		default:
			throw common::Error<Error::InvalidReportType> {};
		}

		if (!m_report_size.has_value())
			throw common::Error<Error::MissingReportSize> {};

		if (!m_report_count.has_value())
			throw common::Error<Error::MissingReportCount> {};

		Report report {
			type,
			m_report_id,
			m_report_size.value(),
			m_report_count.value(),
			m_usages,
		};

		this->reset_local();
		return report;
	}
};

} // namespace iptsd::hid

#endif // IPTSD_HID_STATE_HPP
