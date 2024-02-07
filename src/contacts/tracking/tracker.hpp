// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_TRACKING_TRACKER_HPP
#define IPTSD_CONTACTS_TRACKING_TRACKER_HPP

#include "../contact.hpp"
#include "distances.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>

#include <algorithm>
#include <iterator>
#include <vector>

namespace iptsd::contacts::tracking {

template <class T>
class Tracker {
public:
	static_assert(std::is_floating_point_v<T>);

private:
	// The last frame.
	std::vector<Contact<T>> m_last {};

	// The distances between all contacts from the current and the last frame.
	Image<T> m_distances {};

public:
	/*!
	 * Resets the tracker by clearing the stored copy of the last frame.
	 */
	void reset()
	{
		m_last.clear();
	}

	/*!
	 * Runs the contact tracking algorithm over the contacts from the current frame.
	 *
	 * @param[in,out] frame The list of contacts that will be tracked.
	 */
	void track(std::vector<Contact<T>> &frame)
	{
		usize counter = 0;

		// Assign unique indices to all contacts of the current frame.
		for (Contact<T> &contact : frame) {
			contact.index = this->find_new_index(counter);
			counter = contact.index.value() + 1;
		}

		if (!m_last.empty()) {
			const usize min = std::min(frame.size(), m_last.size());

			// Calculate the distances between all contacts from the current and last
			// frame
			distances::calculate(frame, m_last, m_distances);

			// Copy the old indices back for the amount of contacts that can be tracked.
			for (usize i = 0; i < min; i++) {
				Eigen::Index y = 0;
				Eigen::Index x = 0;

				m_distances.minCoeff(&y, &x);

				// Copy the index of the contact
				frame[casts::to_unsigned(x)].index =
					m_last[casts::to_unsigned(y)].index;

				// Invalidate all entries containing these contacts
				m_distances.row(y) = Eigen::NumTraits<T>::infinity();
				m_distances.col(x) = Eigen::NumTraits<T>::infinity();
			}

			m_last.clear();
		}

		// Save a copy of the new data
		std::copy(frame.begin(), frame.end(), std::back_inserter(m_last));
	}

private:
	/*!
	 * Searches for an index that is not already used by a contact from the last frame.
	 *
	 * @param[in] min The new index has to be at least this value.
	 * @return A new unique index that was not used before.
	 */
	[[nodiscard]] usize find_new_index(usize min) const
	{
		while (true) {
			if (!Contact<T>::find_in_frame(min, m_last).has_value())
				return min;

			min++;
		}
	}
};

} // namespace iptsd::contacts::tracking

#endif // IPTSD_CONTACTS_TRACKING_TRACKER_HPP
