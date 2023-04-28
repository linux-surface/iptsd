// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_GENERIC_CONE_HPP
#define IPTSD_CORE_GENERIC_CONE_HPP

#include <common/chrono.hpp>
#include <common/types.hpp>

#include <type_traits>

namespace iptsd::core {

class Cone {
private:
	using clock = chrono::steady_clock;

private:
	clock::time_point m_position_update {};
	clock::time_point m_direction_update {};

	f64 m_x = 0;
	f64 m_y = 0;
	f64 m_dx = 0;
	f64 m_dy = 0;

	f64 m_angle;
	f64 m_distance;

public:
	Cone(const f64 angle, const f64 distance) : m_angle {angle}, m_distance {distance} {};

	/*!
	 * Checks if the cone is alive.
	 *
	 * @return Whether the cone has seen a position update.
	 */
	[[nodiscard]] bool alive() const
	{
		return m_position_update.time_since_epoch() > clock::duration::zero();
	}

	/*!
	 * Checks if the cone is currently active.
	 *
	 * @return Whether the cone has seen a position update in the last 300 milliseconds.
	 */
	[[nodiscard]] bool active() const
	{
		return m_position_update + 300ms > clock::now();
	}

	/*!
	 * Updates the position of the cone.
	 *
	 * Coordinates have to be in physical dimensions (screen size).
	 *
	 * @param[in] x The X coordinate of the new origin of the cone.
	 * @param[in] y The Y coordinate of the new origin of the cone.
	 */
	void update_position(const f64 x, const f64 y)
	{
		m_x = x;
		m_y = y;
		m_position_update = clock::now();
	}

	/*!
	 * Updates the direction the cone is facing in.
	 *
	 * Coordinates have to be in physical dimensions (screen size).
	 *
	 * @param[in] x The X coordinate of the point the cone should face towards.
	 * @param[in] x The Y coordinate of the point the cone should face towards.
	 */
	void update_direction(const f64 x, const f64 y)
	{
		const clock::time_point timestamp = clock::now();

		const auto time_diff = timestamp - m_direction_update;
		const auto diff = chrono::duration_cast<seconds<f64>>(time_diff);

		const f64 weight = std::exp2(-diff.count());
		f64 dist = std::hypot(m_x - x, m_y - y);

		const f64 dx = (x - m_x) / (dist + 1E-6);
		const f64 dy = (y - m_y) / (dist + 1E-6);

		m_dx = weight * m_dx + dx;
		m_dy = weight * m_dy + dy;

		// Normalize cone direction vector
		dist = std::hypot(m_dx, m_dy) + 1E-6;
		m_dx /= dist;
		m_dy /= dist;

		m_direction_update = timestamp;
	}

	/*!
	 * Checks if a point is convered by the cone.
	 *
	 * Coordinates have to be in physical dimensions (screen size).
	 *
	 * @param[in] x The X coordinate of the point that should be checked.
	 * @param[in] y The Y coordinate of the point that should be checked.
	 * @return Whether the point is covered by the cone.
	 */
	[[nodiscard]] bool check(const f64 x, const f64 y) const
	{
		if (!this->active())
			return false;

		const f64 dx = x - m_x;
		const f64 dy = y - m_y;
		const f64 dist = std::hypot(dx, dy);

		if (dist > m_distance)
			return false;

		if (dx * m_dx + dy * m_dy > m_angle * dist)
			return true;

		return false;
	}
};

} // namespace iptsd::core

#endif // IPTSD_CORE_GENERIC_CONE_HPP
