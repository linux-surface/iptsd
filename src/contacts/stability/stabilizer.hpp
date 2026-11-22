// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_STABILITY_STABILIZER_HPP
#define IPTSD_CONTACTS_STABILITY_STABILIZER_HPP

#include "../contact.hpp"
#include "config.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>

#include <gsl/gsl>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iterator>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace iptsd::contacts::stability {

template <class T>
class Stabilizer {
public:
	static_assert(std::is_floating_point_v<T>);
	using TimePoint = std::chrono::steady_clock::time_point;

private:
	struct OneEuroState {
		Vector2<T> x = Vector2<T>::Zero();
		Vector2<T> dx = Vector2<T>::Zero();
		TimePoint time {};
	};

	Config<T> m_config;

	// The last frame.
	std::vector<Contact<T>> m_last {};

	// Per-contact One-Euro filter state indexed by contact.index.
	std::unordered_map<usize, OneEuroState> m_euro {};

public:
	explicit Stabilizer(Config<T> config) : m_config {std::move(config)} {};

	/*!
	 * Resets the stabilizer by clearing the stored previous frames.
	 */
	void reset()
	{
		m_last.clear();
		m_euro.clear();
	}

	/*!
	 * Stabilizes all contacts of a frame.
	 *
	 * @param[in,out] frame The list of contacts to stabilize.
	 */
	void stabilize(std::vector<Contact<T>> &frame)
	{
		this->stabilize(frame, std::chrono::steady_clock::now());
	}

	void stabilize(std::vector<Contact<T>> &frame, const TimePoint now)
	{
		// Drop One-Euro state for contacts that are no longer present.
		for (auto it = m_euro.begin(); it != m_euro.end();) {
			if (!Contact<T>::find_in_frame(it->first, frame).has_value())
				it = m_euro.erase(it);
			else
				++it;
		}

		// Stabilize contacts
		for (Contact<T> &contact : frame)
			this->stabilize_contact(contact, now);

		m_last.clear();

		// Save a copy of the new data
		std::copy(frame.begin(), frame.end(), std::back_inserter(m_last));
	}

private:
	/*!
	 * Stabilize a single contact.
	 *
	 * @param[in,out] contact The contact to stabilize.
	 */
	void stabilize_contact(Contact<T> &contact, const TimePoint now)
	{
		// Contacts that can't be tracked can't be stabilized.
		if (!contact.index.has_value())
			return;

		contact.stable = true;

		const usize index = contact.index.value();
		const auto wrapper = Contact<T>::find_in_frame(index, m_last);

		if (!wrapper.has_value()) {
			if (m_config.one_euro_enabled) {
				m_euro[index] = OneEuroState {
					contact.mean,
					Vector2<T>::Zero(),
					now,
				};
			}

			return;
		}

		const Contact<T> &last = wrapper.value();

		if (m_config.size_threshold.has_value())
			this->stabilize_size(contact, last);

		if (m_config.position_threshold.has_value() || m_config.one_euro_enabled)
			this->stabilize_position(contact, last, now);

		if (m_config.orientation_threshold.has_value())
			this->stabilize_orientation(contact, last);
	}

	/*!
	 * Stabilizes the size of the contact.
	 *
	 * @param[in,out] current The contact to stabilize.
	 * @param[in] last The contact to compare against.
	 */
	void stabilize_size(Contact<T> &current, const Contact<T> &last) const
	{
		if (!m_config.size_threshold.has_value())
			return;

		const Vector2<T> thresh = m_config.size_threshold.value();
		const Vector2<T> delta = (current.size - last.size).cwiseAbs();

		/*
		 * If the size is increasing too slow, discard the change.
		 * If the size is increasing too fast, mark it as unstable (we can't stabilize it).
		 * Otherwise, don't change the size.
		 */

		if (delta.x() < thresh.x())
			current.size.x() = last.size.x();
		else if (delta.x() > thresh.y())
			current.stable = false;

		if (delta.y() < thresh.x())
			current.size.y() = last.size.y();
		else if (delta.y() > thresh.y())
			current.stable = false;
	}

	static T smoothing_factor(const T dt, const T cutoff)
	{
		constexpr T two_pi = casts::to<T>(2) * gsl::narrow_cast<T>(M_PI);
		const T r = two_pi * cutoff * dt;
		return r / (r + casts::to<T>(1));
	}

	/*!
	 * Stabilizes the position of the contact.
	 *
	 * @param[in,out] current The contact to stabilize.
	 * @param[in] last The contact to compare against.
	 */
	void stabilize_position(Contact<T> &current, const Contact<T> &last, const TimePoint now)
	{
		const Vector2<T> delta = current.mean - last.mean;
		const T distance = std::hypot(delta.x(), delta.y());

		if (m_config.position_threshold.has_value()) {
			const Vector2<T> thresh = m_config.position_threshold.value();

			if (distance > thresh.y()) {
				current.stable = false;
				return;
			}

			if (!m_config.one_euro_enabled && distance < thresh.x()) {
				current.mean = last.mean;
				return;
			}
		}

		if (!m_config.one_euro_enabled)
			return;

		const usize index = current.index.value();
		const auto it = m_euro.find(index);
		if (it == m_euro.end()) {
			m_euro[index] = OneEuroState {current.mean, Vector2<T>::Zero(), now};
			return;
		}

		OneEuroState &state = it->second;
		const auto dt_us =
			std::chrono::duration_cast<std::chrono::microseconds>(now - state.time)
				.count();
		const T dt = std::clamp(casts::to<T>(dt_us) / casts::to<T>(1000000),
		                        gsl::narrow_cast<T>(0.001),
		                        gsl::narrow_cast<T>(0.1));

		const Vector2<T> dx = (current.mean - state.x) / dt;
		const T alpha_d = smoothing_factor(dt, m_config.one_euro_d_cutoff);
		state.dx = state.dx + alpha_d * (dx - state.dx);

		// Compute speed in physical units (cm/s) so min_cutoff/beta are device-independent.
		const T speed = state.dx.cwiseProduct(m_config.physical_size).norm();
		const T cutoff = m_config.one_euro_min_cutoff + (m_config.one_euro_beta * speed);
		const T alpha = smoothing_factor(dt, cutoff);

		state.x = state.x + alpha * (current.mean - state.x);
		state.time = now;
		current.mean = state.x;
	}

	void stabilize_orientation(Contact<T> &current, const Contact<T> &last) const
	{
		if (!m_config.orientation_threshold.has_value())
			return;

		const T aspect = current.size.maxCoeff() / current.size.minCoeff();

		/*
		 * If the aspect ratio is too small, the orientation cannot be determined
		 * in a stable way. To prevent errors, we set it to 0 in this case.
		 *
		 * TODO: Check if there is a better way to signal this (make orientation optional,
		 * and / or applying the last stable value).
		 */
		if (aspect < 1.1) {
			current.orientation = 0;
			return;
		}

		const Vector2<T> thresh = m_config.orientation_threshold.value();

		const T max = current.normalized ? casts::to<T>(1) : gsl::narrow_cast<T>(M_PI);

		// The angle difference in both directions.
		const T d1 = std::abs(current.orientation - last.orientation);
		const T d2 = max - d1;

		// Pick the smaller difference to properly handle going from 0° to 179°.
		const T delta = std::min(d1, d2);

		/*
		 * If the angle is changing too slow, discard the orientation change.
		 * If the angle is changing too fast, mark it as unstable (we can't stabilize it).
		 * Otherwise, don't change the orientation.
		 */

		if (delta < thresh.x())
			current.orientation = last.orientation;
		else if (delta > thresh.y())
			current.stable = false;
	}
};

} // namespace iptsd::contacts::stability

#endif // IPTSD_CONTACTS_STABILITY_STABILIZER_HPP
