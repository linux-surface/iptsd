// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_STABILITY_CHECKER_HPP
#define IPTSD_CONTACTS_STABILITY_CHECKER_HPP

#include "../contact.hpp"
#include "config.hpp"

#include <common/casts.hpp>
#include <common/constants.hpp>
#include <common/types.hpp>

#include <algorithm>
#include <deque>
#include <type_traits>
#include <vector>

namespace iptsd::contacts::stability {

template <class T>
class Checker {
public:
	static_assert(std::is_floating_point_v<T>);

private:
	Config<T> m_config;

	// The last n frames, with n being m_config.temporal_window.
	std::deque<std::vector<Contact<T>>> m_frames;

public:
	Checker(Config<T> config)
		: m_config {config}
		, m_frames {std::max(config.temporal_window, casts::to<usize>(2)) - 1} {};

	/*!
	 * Resets the checker by clearing the stored copies of the last frames.
	 */
	void reset()
	{
		for (auto &frame : m_frames)
			frame.clear();
	}

	/*!
	 * Checks the stability for all contacts of a frame.
	 *
	 * @param[in,out] frame The list of contacts to check.
	 */
	void check(std::vector<Contact<T>> &frame)
	{
		for (Contact<T> &contact : frame)
			contact.stable = this->check_contact(contact, frame);

		auto &nf = m_frames.front();
		nf.clear();

		// Store a copy of the new data
		std::copy(frame.begin(), frame.end(), std::back_inserter(nf));

		m_frames.pop_front();
		m_frames.push_back(nf);
	}

private:
	/*!
	 * Checks a single contact.
	 *
	 * @param[in] contact The contact to check.
	 * @param[in] stability An optional validity checker to enable additional stability checks.
	 * @return Whether the contact is valid.
	 */
	[[nodiscard]] bool check_contact(const Contact<T> &contact,
					 const std::vector<Contact<T>> &frame) const
	{
		const bool should_temp = m_config.check_temporal_stability;
		const bool should_dist = m_config.distance_threshold.has_value();
		const bool should_size = m_config.size_difference_threshold.has_value();
		const bool should_move = m_config.movement_limits.has_value();

		if (should_dist && !this->check_distance(contact, frame))
			return false;

		// Contacts that can't be tracked are considered temporally stable.
		if (!contact.index.has_value())
			return true;

		if (m_config.temporal_window < 2)
			return true;

		const usize index = contact.index.value();
		Contact<T> current = contact;

		// Iterate over the last frames and find the contact with the same index
		for (auto itr = m_frames.crbegin(); itr != m_frames.crend(); itr++) {
			const auto wrapper = Contact<T>::find_in_frame(index, *itr);

			if (!wrapper.has_value())
				return !should_temp;

			const Contact<T> &last = wrapper.value();

			if (should_size && !this->check_size(current, last))
				return false;

			if (should_move && !this->check_movement(current, last))
				return false;

			current = last;
		}

		return true;
	}

	/*!
	 * Checks if the contact is close to any invalid contacts.
	 *
	 * @param[in] contact The contact to check.
	 * @param[in] frame The other contacts that the contact will be compared against.
	 * @return Whether the contact is far enough away from any invalid contacts.
	 */
	[[nodiscard]] bool check_distance(const Contact<T> &contact,
					  const std::vector<Contact<T>> &frame) const
	{
		if (!contact.index.has_value() || !m_config.distance_threshold.has_value())
			return true;

		const usize index = contact.index.value();
		const T distance_thresh = m_config.distance_threshold.value();

		return std::all_of(frame.cbegin(), frame.cend(), [&](const auto &v) {
			if (v.index == index)
				return true;

			if (v.valid.value_or(true))
				return true;

			/*
			 * Assumption: All contacts are perfect circles. The radius is major / 2.
			 * This covers more area than neccessary, but will make the code easier.
			 */

			const Vector2<T> delta = contact.mean - v.mean;
			const T distance = std::hypot(delta.x(), delta.y());

			const T difference =
				distance - (contact.size.maxCoeff() / 2) - (v.size.maxCoeff() / 2);

			return difference >= distance_thresh;
		});
	}

	/*!
	 * Checks the stability of the size of the contact.
	 *
	 * @param[in] current The contact to check.
	 * @param[in] last The contact to compare against.
	 * @return Whether the size difference of the contact is within the allowed range.
	 */
	[[nodiscard]] bool check_size(const Contact<T> &current, const Contact<T> &last) const
	{
		if (!m_config.size_difference_threshold.has_value())
			return true;

		const T size_thresh = m_config.size_difference_threshold.value();
		const Vector2<T> delta = (current.size - last.size).cwiseAbs();

		// Is the contact rapidly changing its size?
		return (delta.array() <= size_thresh).all();
	}

	/*!
	 * Checks the stability of the movement of the contact.
	 *
	 * @param[in] current The contact to check.
	 * @param[in] last The contact to compare against.
	 * @return Whether the movement of the contact is within the allowed range.
	 */
	[[nodiscard]] bool check_movement(const Contact<T> &current, const Contact<T> &last) const
	{
		if (!m_config.movement_limits.has_value())
			return true;

		const Vector2<T> limit = m_config.movement_limits.value();

		const Vector2<T> delta = current.mean - last.mean;
		const T distance = std::hypot(delta.x(), delta.y());

		// Is the contact moving too fast or too slow?
		return distance >= limit.x() || distance <= limit.y();
	}
};

} // namespace iptsd::contacts::stability

#endif // IPTSD_CONTACTS_STABILITY_CHECKER_HPP
