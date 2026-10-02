// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_TRACKING_TRACKER_HPP
#define IPTSD_CONTACTS_TRACKING_TRACKER_HPP

#include "../contact.hpp"
#include "config.hpp"
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
	Config<T> m_config;

	// The last frame, including contacts that are retained after they disappeared.
	std::vector<Contact<T>> m_last {};

	// For how many frames each contact from the last frame has not been detected.
	std::vector<usize> m_missed {};

	// Whether a contact from the last frame was found in the current frame.
	std::vector<bool> m_matched {};

	// The distances between all contacts from the current and the last frame.
	Image<T> m_distances {};

public:
	Tracker(const Config<T> &config) : m_config {config} {};

	/*!
	 * Resets the tracker by clearing the stored copy of the last frame.
	 */
	void reset()
	{
		m_last.clear();
		m_missed.clear();
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

		m_matched.assign(m_last.size(), false);

		if (!m_last.empty() && !frame.empty()) {
			// Calculate the distances between all contacts from the current and last
			// frame
			distances::calculate(frame, m_last, m_distances);

			// Retained contacts can only be picked up again close to where they were
			// lost
			for (usize i = 0; i < m_last.size(); i++) {
				if (m_missed[i] == 0)
					continue;

				const Contact<T> &retained = m_last[i];

				for (usize j = 0; j < frame.size(); j++) {
					const Vector2<T> delta = frame[j].mean - retained.mean;
					const T distance =
						delta.cwiseProduct(m_config.retain_scale).norm();

					if (distance > m_config.retain_distance) {
						m_distances(casts::to_eigen(i),
						            casts::to_eigen(j)) =
							Eigen::NumTraits<T>::infinity();
					}
				}
			}

			// Copy the old indices back for all contacts that can be tracked.
			while (true) {
				Eigen::Index y = 0;
				Eigen::Index x = 0;

				const T distance = m_distances.minCoeff(&y, &x);

				if (distance == Eigen::NumTraits<T>::infinity())
					break;

				// Copy the index of the contact
				frame[casts::to_unsigned(x)].index =
					m_last[casts::to_unsigned(y)].index;

				m_matched[casts::to_unsigned(y)] = true;

				// Invalidate all entries containing these contacts
				m_distances.row(y) = Eigen::NumTraits<T>::infinity();
				m_distances.col(x) = Eigen::NumTraits<T>::infinity();
			}
		}

		const usize detected = frame.size();

		// Keep contacts that were not detected alive for a few frames, by repeating them.
		for (usize i = 0; i < m_last.size(); i++) {
			if (m_matched[i] || m_missed[i] >= m_config.retain_frames)
				continue;

			frame.push_back(m_last[i]);
			m_missed.push_back(m_missed[i] + 1);
		}

		// The counters of the retained contacts were appended after the old ones
		m_missed.erase(m_missed.begin(),
		               m_missed.begin() + casts::to_signed(m_last.size()));
		m_missed.insert(m_missed.begin(), detected, 0);

		// Save a copy of the new data
		m_last.clear();
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
