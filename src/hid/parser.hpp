// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_PARSER_HPP
#define IPTSD_HID_PARSER_HPP

#include "collection.hpp"
#include "descriptor.hpp"
#include "field.hpp"
#include "protocol/item.hpp"
#include "report.hpp"

#include <common/casts.hpp>
#include <common/reader.hpp>
#include <common/types.hpp>

#include <optional>
#include <stack>

namespace iptsd::hid {
namespace impl {

struct GlobalState {
	u32 usage_page = 0;
	std::optional<u8> report_id = std::nullopt;

	u32 logical_min = 0;
	u32 logical_max = 0;
	std::optional<u32> physical_min = std::nullopt;
	std::optional<u32> physical_max = std::nullopt;

	std::optional<u32> unit = std::nullopt;
	std::optional<u32> unit_exp = std::nullopt;

	u32 report_count = 0;
	u32 report_size = 0;
};

struct LocalState {
	u32 usage = 0;

	std::optional<u32> usage_min = std::nullopt;
	std::optional<u32> usage_max = std::nullopt;
};

} // namespace impl

class Parser {
private:
	std::stack<impl::GlobalState> m_global_stack {};
	std::stack<Collection> m_collection_stack {};

	impl::GlobalState m_global {};
	impl::LocalState m_local {};
	Collection m_collection {};

public:
	/*!
	 * Parses a binary report descriptor.
	 *
	 * @param[in] buffer The buffer containing the binary report descriptor.
	 * @param[out] desc The structure that the parsed report descriptor will be saved to.
	 */
	void parse(const gsl::span<u8> buffer, Descriptor &desc)
	{
		Reader reader {buffer};

		while (reader.size() > 0)
			this->parse_item(reader);

		desc.reports = m_collection.reports;
		desc.collections = m_collection.collections;
	}

	/*!
	 * Parses a binary report descriptor.
	 *
	 * @param[in] buffer The buffer containing the binary report descriptor.
	 * @return A structure representing the parsed report descriptor.
	 */
	Descriptor parse(const gsl::span<u8> buffer)
	{
		Descriptor desc {};
		this->parse(buffer, desc);
		return desc;
	}

private:
	/*!
	 * Reads a short item from the binary report descriptor and updates the parser state.
	 *
	 * The short item format packs the item size, type, and tag into the first byte.
	 * The first byte may be followed by 0, 1, 2, or 4 optional data bytes depending on the
	 * size of the data.
	 *
	 * While the HID specification defines the format for long items, it doesn't define
	 * anything actually using it.
	 *
	 * @param[in] reader The chunk of data allocated to the report descriptor.
	 */
	void parse_item(Reader &reader)
	{
		const auto header = reader.read<protocol::item::Header>();

		u32 payload = 0;

		switch (header.size) {
		case protocol::item::Size::OneByte:
			payload = reader.read<u8>();
			break;
		case protocol::item::Size::TwoBytes:
			payload = reader.read<u16>();
			break;
		case protocol::item::Size::FourBytes:
			payload = reader.read<u32>();
			break;
		case protocol::item::Size::NoPayload:
			break;
		}

		switch (header.tag) {
		case protocol::item::Tag::Input:
		case protocol::item::Tag::Output:
		case protocol::item::Tag::Feature:
			this->create_field(header.tag);
			break;
		case protocol::item::Tag::Collection:
			this->open_collection(payload);
			break;
		case protocol::item::Tag::EndCollection:
			this->close_collection();
			break;

		// Global
		case protocol::item::Tag::UsagePage:
			m_global.usage_page = payload << 16;
			break;
		case protocol::item::Tag::LocalMinimum:
			m_global.logical_min = payload;
			break;
		case protocol::item::Tag::LocalMaximum:
			m_global.logical_max = payload;
			break;
		case protocol::item::Tag::PhysicalMinimum:
			m_global.physical_min = payload;
			break;
		case protocol::item::Tag::PhysicalMaximum:
			m_global.physical_max = payload;
			break;
		case protocol::item::Tag::UnitExponent:
			m_global.unit_exp = payload;
			break;
		case protocol::item::Tag::Unit:
			m_global.unit = payload;
			break;
		case protocol::item::Tag::ReportSize:
			m_global.report_size = payload;
			break;
		case protocol::item::Tag::ReportID:
			m_global.report_id = casts::to<u8>(payload);
			break;
		case protocol::item::Tag::ReportCount:
			m_global.report_count = payload;
			break;
		case protocol::item::Tag::Push:
			this->push_global();
			break;
		case protocol::item::Tag::Pop:
			this->pop_global();
			break;

		// Local
		case protocol::item::Tag::Usage:
			m_local.usage = this->extended_usage(header.size, payload);
			break;
		case protocol::item::Tag::UsageMinimum:
			m_local.usage_min = this->extended_usage(header.size, payload);
			break;
		case protocol::item::Tag::UsageMaximum:
			m_local.usage_max = this->extended_usage(header.size, payload);
			break;

		// Not implemented
		case protocol::item::Tag::DesignatorIndex:
		case protocol::item::Tag::DesignatorMinimum:
		case protocol::item::Tag::DesignatorMaximum:
		case protocol::item::Tag::StringIndex:
		case protocol::item::Tag::StringMinimum:
		case protocol::item::Tag::StringMaximum:
		case protocol::item::Tag::Delimiter:
			break;
		}
	}

	/*!
	 * If the payload size = 3 then the item is interpreted as a 32 bit unsigned value where
	 * the high order 16 bits defines the Usage Page and the low order 16 bits defines the
	 * Usage ID. 32 bit usage items that define both the Usage Page and Usage ID are often
	 * referred to as “Extended” Usages.
	 *
	 * If the payload size = 1 or 2 then the Usage is interpreted as an unsigned value that
	 * selects a Usage ID on the currently defined Usage Page. When the parser encounters a
	 * main item it concatenates the last declared Usage Page with a Usage to form a complete
	 * usage value. Extended usages can be used to override the currently defined Usage Page
	 * for individual usages.
	 */
	[[nodiscard]] u32 extended_usage(const protocol::item::Size size, const u32 payload) const
	{
		if (size != protocol::item::Size::FourBytes)
			return m_global.usage_page | payload;

		return payload;
	}

	/*!
	 * Pushes the current global state to the stack.
	 */
	void push_global()
	{
		m_global_stack.push(m_global);
	}

	/*!
	 * Restores the global state from the stack.
	 */
	void pop_global()
	{
		m_global = m_global_stack.top();
		m_global_stack.pop();
	}

	/*!
	 * Resets the local state after it was consumed.
	 */
	void reset_local()
	{
		m_local = impl::LocalState {};
	}

	/*!
	 * Saves the currently active collection to the stack and creates a new one.
	 */
	void open_collection(const u32 payload)
	{
		m_collection_stack.push(m_collection);

		m_collection = Collection {};
		m_collection.type = protocol::collection::parse_type(casts::to<u8>(payload));
		m_collection.usage = m_local.usage;

		this->reset_local();
	}

	/*!
	 * Restores the parent collection from the stack.
	 * The current collection will be finalized and added as a child to the restored parent.
	 */
	void close_collection()
	{
		Collection parent = m_collection_stack.top();
		m_collection_stack.pop();

		parent.collections.push_back(m_collection);
		m_collection = parent;
	}

	/*!
	 * Searches for a report with the current Report ID and the right type.
	 * If no such report exists, a new one is created in the current collection.
	 */
	Report &create_or_get_report(const protocol::item::Tag tag)
	{
		Report::Type type {};

		switch (tag) {
		case protocol::item::Tag::Input:
			type = Report::Type::Input;
			break;
		case protocol::item::Tag::Output:
			type = Report::Type::Output;
			break;
		case protocol::item::Tag::Feature:
			type = Report::Type::Feature;
			break;
		default:
			// Should never happen
			break;
		}

		for (Report &report : m_collection.reports) {
			if (report.report_id != m_global.report_id)
				continue;

			if (report.type != type)
				continue;

			return report;
		}

		Report report {};
		report.type = type;
		report.report_id = m_global.report_id;

		m_collection.reports.push_back(report);
		return m_collection.reports.back();
	}

	/*!
	 * Creates a new field in the current report.
	 */
	void create_field(const protocol::item::Tag tag)
	{
		Report &report = this->create_or_get_report(tag);

		Field field {};
		field.count = m_global.report_count;
		field.size = m_global.report_size;

		field.usage = m_local.usage;
		field.usage_min = m_local.usage_min;
		field.usage_max = m_local.usage_max;

		field.logical_min = m_global.logical_min;
		field.logical_max = m_global.logical_max;
		field.physical_min = m_global.physical_min;
		field.physical_max = m_global.physical_max;

		field.unit = m_global.unit;
		field.unit_exp = m_global.unit_exp;

		report.fields.push_back(field);
		this->reset_local();
	}
};

/*!
 * Parses a binary report descriptor.
 *
 * @param[in] buffer The buffer containing the binary report descriptor.
 * @param[out] desc The structure that the parsed report descriptor will be saved to.
 */
inline void parse(const gsl::span<u8> buffer, Descriptor &desc)
{
	Parser {}.parse(buffer, desc);
}

/*!
 * Parses a binary report descriptor.
 *
 * @param[in] buffer The buffer containing the binary report descriptor.
 * @return A structure representing the parsed report descriptor.
 */
inline Descriptor parse(const gsl::span<u8> buffer)
{
	return Parser {}.parse(buffer);
}

} // namespace iptsd::hid

#endif // IPTSD_HID_PARSER_HPP
