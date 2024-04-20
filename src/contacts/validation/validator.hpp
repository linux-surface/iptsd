// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_VALIDATION_VALIDATOR_HPP
#define IPTSD_CONTACTS_VALIDATION_VALIDATOR_HPP

#include "../contact.hpp"
#include "config.hpp"

#include <common/types.hpp>

#include <type_traits>
#include <vector>

namespace iptsd::contacts::validation {

template <class T>
class Validator {
public:
	static_assert(std::is_floating_point_v<T>);

private:
	// The config for the validity checking phase.
	Config<T> m_config;

	// The last frame.
	std::vector<Contact<T>> m_last {};

public:
	Validator(Config<T> config) : m_config {std::move(config)} {};

	/*!
	 * Resets the validator by clearing the stored copy of the last frame.
	 */
	void reset()
	{
		m_last.clear();
	}

	/*!
	 * Checks the validity for all contacts of a frame.
	 *
	 * @param[in,out] frame The list of contacts to validate.
	 */
	void validate(std::vector<Contact<T>> &frame)
	{
		for (Contact<T> &contact : frame)
			contact.valid = this->check_contact(contact);

		m_last.clear();

		// Save a copy of the new data
		std::copy(frame.begin(), frame.end(), std::back_inserter(m_last));
	}

private:
	/*!
	 * Checks a single contact.
	 *
	 * @param[in] contact The contact to check.
	 * @return Whether the contact is valid.
	 */
	bool check_contact(const Contact<T> &contact)
	{
		// Don't invalidate unstable contacts
		if (!contact.stable.value_or(true))
			return true;

		/*
		 * If the state should be tracked and the contact was invalid in the
		 * last frame, it is also invalid in the current frame.
		 */
		if (m_config.track_validity && !this->check_temporal(contact))
			return false;

		// Only do the size check if it is enabled
		if (m_config.size_limits.has_value() && !this->check_size(contact))
			return false;

		// Only do the aspect check if it is enabled
		if (m_config.aspect_limits.has_value() && !this->check_aspect(contact))
			return false;

		return true;
	}

	/*!
	 * Checks the temporal validity of a contact.
	 *
	 * @param[in] contact The contact to check.
	 * @return Whether the contact was valid in the last frame.
	 */
	bool check_temporal(const Contact<T> &contact)
	{
		// Contacts that can't be tracked are considered temporally valid.
		if (!contact.index.has_value())
			return true;

		const auto wrapper = Contact<T>::find_in_frame(contact.index.value(), m_last);

		if (!wrapper.has_value())
			return true;

		const Contact<T> &last = wrapper.value();

		if (!last.valid.has_value())
			return true;

		return last.valid.value();
	}

	/*!
	 * Checks the size of a contact.
	 *
	 * @param[in] contact The contact to check.
	 * @return Whether the size of the contact is within the valid range.
	 */
	bool check_size(const Contact<T> &contact)
	{
		if (!m_config.size_limits.has_value())
			return true;

		const Vector2<T> &limit = m_config.size_limits.value();
		const Vector2<T> &size = contact.size;

		const T major = size.maxCoeff();
		return major >= limit.minCoeff() && major <= limit.maxCoeff();
	}

	/*!
	 * Checks the aspect ratio of a contact.
	 *
	 * @param[in] contact The contact to check.
	 * @return Whether the aspect ratio of the contact is within the valid range.
	 */
	bool check_aspect(const Contact<T> &contact)
	{
		if (!m_config.aspect_limits.has_value())
			return true;

		const Vector2<T> &limit = m_config.aspect_limits.value();
		const Vector2<T> &size = contact.size;

		const T major = size.maxCoeff();
		const T minor = size.minCoeff();

		const T aspect = major / minor;
		return aspect >= limit.minCoeff() && aspect <= limit.maxCoeff();
	}
};

} // namespace iptsd::contacts::validation

#endif // IPTSD_CONTACTS_VALIDATION_VALIDATOR_HPP
