/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_FINDER_HPP
#define IPTSD_CONTACTS_FINDER_HPP

#include "config.hpp"
#include "contact.hpp"
#include "detection/detector.hpp"
#include "stability/checker.hpp"
#include "tracking/tracker.hpp"
#include "validation/validator.hpp"

#include <common/types.hpp>

#include <type_traits>
#include <vector>

namespace iptsd::contacts {

template <class T, class TFit = T>
class Finder {
public:
	static_assert(std::is_floating_point_v<T>);
	static_assert(std::is_floating_point_v<TFit>);

private:
	// Detects contacts in a capacitive heatmap.
	detection::Detector<T, TFit> m_detector;

	// Tracks contacts over multiple frames.
	tracking::Tracker<T> m_tracker {};

	// Validates size and aspect ratio of contacts.
	validation::Validator<T> m_validator;

	// Check the temporal stability of contacts.
	stability::Checker<T> m_stability;

public:
	Finder(Config<T> config)
		: m_detector {config.detection}
		, m_validator {config.validation}
		, m_stability {config.stability} {};

	/*!
	 * Resets the contact finder by clearing all stored previous frames.
	 */
	void reset()
	{
		m_tracker.reset();
		m_validator.reset();
		m_stability.reset();
	}

	/*!
	 * Extracts contacts from a capacitive heatmap.
	 *
	 * After the initial detection phase, every contact will be assigned a uniqe
	 * index that identifies them over multiple consecutive frames.
	 *
	 * Then the size and aspect ratio of the contact is validated, and it is
	 * checked if the changes to the contact over the last frames have been stable.
	 *
	 * @param[in] heatmap The capacitive heatmap to process.
	 * @param[out] contacts The list of found contacts.
	 */
	template <int Rows, int Cols>
	void find(const ImageBase<T, Rows, Cols> &heatmap, std::vector<Contact<T>> &contacts)
	{
		m_detector.detect(heatmap, contacts);
		m_tracker.track(contacts);
		m_validator.validate(contacts);
		m_stability.check(contacts);
	}
};

} // namespace iptsd::contacts

#endif // IPTSD_CONTACTS_FINDER_HPP
