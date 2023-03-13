// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_DAEMON_CONE_HPP
#define IPTSD_DAEMON_CONE_HPP

#include <common/types.hpp>

#include <chrono>

namespace iptsd::daemon {

class Cone {
private:
	using clock = std::chrono::system_clock;

	clock::time_point m_position_update {};
	clock::time_point m_direction_update {};

	f32 m_x = 0;
	f32 m_y = 0;
	f32 m_dx = 0;
	f32 m_dy = 0;

	f32 m_angle;
	f32 m_distance;

public:
	Cone(f32 angle, f32 distance) : m_angle {angle}, m_distance {distance} {};

	/*!
	 * Checks if the cone is alive.
	 *
	 * @return Whether the cone has seen a position update.
	 */
	[[nodiscard]] bool alive() const
	{
		return m_position_update > clock::from_time_t(0);
	}

	/*!
	 * Checks if the cone is currently active.
	 *
	 * @return Whether the cone has seen a position update in the last 300 milliseconds.
	 */
	[[nodiscard]] bool active() const
	{
		return m_position_update + std::chrono::milliseconds {300} > clock::now();
	}

	/*!
	 * Updates the position of the cone.
	 *
	 * Coordinates have to be in physical dimensions (screen size).
	 *
	 * @param[in] x The X coordinate of the new origin of the cone.
	 * @param[in] y The Y coordinate of the new origin of the cone.
	 */
	void update_position(f32 x, f32 y)
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
	void update_direction(f32 x, f32 y)
	{
		const clock::time_point timestamp = clock::now();

		const auto time_diff = timestamp - m_direction_update;
		const auto diff = std::chrono::duration_cast<std::chrono::seconds>(time_diff);

		const f32 weight = std::exp2(-gsl::narrow<f32>(diff.count()));
		f32 dist = std::hypot(m_x - x, m_y - y);

		const f32 dx = (x - m_x) / (dist + 1E-6f);
		const f32 dy = (y - m_y) / (dist + 1E-6f);

		m_dx = weight * m_dx + dx;
		m_dy = weight * m_dy + dy;

		// Normalize cone direction vector
		dist = std::hypot(m_dx, m_dy) + 1E-6f;
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
	bool check(f32 x, f32 y)
	{
		if (!this->active())
			return false;

		const f32 dx = x - m_x;
		const f32 dy = y - m_y;
		const f32 dist = std::hypot(dx, dy);

		if (dist > m_distance)
			return false;

		if (dx * m_dx + dy * m_dy > m_angle * dist)
			return true;

		return false;
	}
};

} // namespace iptsd::daemon

#endif // IPTSD_DAEMON_CONE_HPP
