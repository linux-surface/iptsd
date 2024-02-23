// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_COLLECTION_HPP
#define IPTSD_HID_COLLECTION_HPP

#include "protocol/collection.hpp"
#include "report.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>

#include <vector>

namespace iptsd::hid {

/*!
 * A collection is a group of reports that are related to each other. It is assigned a Usage value
 * to clearly define its purpose. Collections can be nested, but do not define data. They are a
 * purely for organizing the report descriptor.
 *
 * For example, a device that provides both keyboard and mouse functionality would put them
 * into two different collections.
 */
class Collection {
public:
	protocol::collection::Type type;
	u32 usage;

	std::vector<Report> reports {};
	std::vector<Collection> collections {};

public:
	/*!
	 * Checks if a Usage applies to this collection.
	 *
	 * @param page The Usage Page to check against (upper 16 bit of the Usage tag).
	 * @param id The Usage ID to check against (lower 16 bit of the Usage tag).
	 * @return Whether the collection is marked with the given Usage.
	 */
	[[nodiscard]] bool has_usage(const u16 usage_page, const u16 usage_id) const
	{
		return this->usage == ((casts::to<u32>(usage_page) << 16) | usage_id);
	}

	/*!
	 * Searches for reports in the collection.
	 *
	 * @param[in] selector A function that decides whether a report should be returned.
	 * @param[out] out The vector that all found reports will be added to.
	 */
	void find_reports(const std::function<bool(const Report &)> &selector,
	                  std::vector<Report> &out) const
	{
		for (const Report &report : this->reports) {
			if (selector(report))
				out.push_back(report);
		}

		for (const Collection &coll : this->collections)
			coll.find_reports(selector, out);
	}

	/*!
	 * Searches for collections in the collection.
	 *
	 * @param[in] selector A function that decides whether a collection should be returned.
	 * @param[out] out The vector that all found collections will be added to.
	 */
	void find_collections(const std::function<bool(const Collection &)> &selector,
	                      std::vector<Collection> &out) const
	{
		for (const Collection &collection : this->collections) {
			if (selector(collection))
				out.push_back(collection);

			collection.find_collections(selector, out);
		}
	}
};

} // namespace iptsd::hid

#endif // IPTSD_HID_COLLECTION_HPP
