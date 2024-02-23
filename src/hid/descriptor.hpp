// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_DESCRIPTOR_HPP
#define IPTSD_HID_DESCRIPTOR_HPP

#include "collection.hpp"
#include "report.hpp"

#include <common/reader.hpp>
#include <common/types.hpp>

#include <vector>

namespace iptsd::hid {

/*!
 * The report descriptor of a device defines a list of data structures and how their contents
 * should be interpreted. These structures are called reports and can be used to communicate
 * between an application and a device.
 */
class Descriptor {
public:
	std::vector<Report> reports {};
	std::vector<Collection> collections {};

public:
	/*!
	 * Searches for reports in the report descriptor.
	 *
	 * @param[in] selector A function that decides whether a report should be returned.
	 * @returns A vector with all found reports.
	 */
	[[nodiscard]] std::vector<Report>
	find_reports(const std::function<bool(const Report &)> &selector) const
	{
		std::vector<Report> out {};

		for (const Report &report : this->reports) {
			if (selector(report))
				out.push_back(report);
		}

		for (const Collection &collection : this->collections)
			collection.find_reports(selector, out);

		return out;
	}

	/*!
	 * Searches for reports in the report descriptor.
	 *
	 * @param[in] selector A function that decides whether a report should be returned.
	 * @returns The first found report, or @ref std::nullopt.
	 */
	[[nodiscard]] std::optional<Report>
	find_report(const std::function<bool(const Report &)> &selector) const
	{
		const std::vector<Report> reports = this->find_reports(selector);

		if (reports.empty())
			return std::nullopt;

		return reports.front();
	}

	/*!
	 * Searches for reports in the report descriptor.
	 *
	 * @param[in] selector A function that decides whether a report should be returned.
	 * @returns Whether any report has been found.
	 */
	[[nodiscard]] bool has_report(const std::function<bool(const Report &)> &selector) const
	{
		return this->find_report(selector).has_value();
	}

	/*!
	 * Searches for collections in the report descriptor.
	 *
	 * @param[in] selector A function that decides whether a collection should be returned.
	 * @returns A vector with all found collections.
	 */
	[[nodiscard]] std::vector<Collection>
	find_collections(const std::function<bool(const Collection &)> &selector) const
	{
		std::vector<Collection> out {};

		for (const Collection &collection : this->collections) {
			if (selector(collection))
				out.push_back(collection);

			collection.find_collections(selector, out);
		}

		return out;
	}

	/*!
	 * Searches for collections in the report descriptor.
	 *
	 * @param[in] selector A function that decides whether a collection should be returned.
	 * @returns The first found collection, or @ref std::nullopt.
	 */
	[[nodiscard]] std::optional<Collection>
	find_collection(const std::function<bool(const Collection &)> &selector) const
	{
		const std::vector<Collection> collections = this->find_collections(selector);

		if (collections.empty())
			return std::nullopt;

		return collections.front();
	}

	/*!
	 * Searches for collections in the report descriptor.
	 *
	 * @param[in] selector A function that decides whether a collection should be returned.
	 * @returns Whether any collection has been found.
	 */
	[[nodiscard]] bool
	has_collection(const std::function<bool(const Collection &)> &selector) const
	{
		return this->find_collection(selector).has_value();
	}
};

} // namespace iptsd::hid

#endif // IPTSD_HID_DESCRIPTOR_HPP
