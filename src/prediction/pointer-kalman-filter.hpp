// SPDX-License-Identifier: Apache-2.0

#ifndef IPTSD_PREDICTION_POINTER_KALMAN_FILTER_HPP
#define IPTSD_PREDICTION_POINTER_KALMAN_FILTER_HPP

#include "kalman-filter.hpp"

#include <common/types.hpp>

namespace iptsd::prediction {

/*
 * Class that independently applies the Kalman Filter to each axis of the pen.
 */
class PointerKalmanFilter {
private:
	KalmanFilter m_x_kalman;
	KalmanFilter m_y_kalman;
	KalmanFilter m_p_kalman;

	Vector2<f64> m_position {};
	Vector2<f64> m_velocity {};
	Vector2<f64> m_acceleration {};
	Vector2<f64> m_jank {};

	f64 m_pressure = 0;
	f64 m_pressure_change = 0;

	usize m_iterations = 0;

	Matrix<f64, 1, 1> m_new_x {};
	Matrix<f64, 1, 1> m_new_y {};
	Matrix<f64, 1, 1> m_new_p {};

public:
	/*!
	 * @param sigmaProcess lower value = more filtering
	 * @param sigmaMeasurement higher value = more filtering
	 */
	PointerKalmanFilter(const f64 sigma_process, const f64 sigma_measurement)
		: m_x_kalman {create_axis_kalman_filter(sigma_process, sigma_measurement)}
		, m_y_kalman {create_axis_kalman_filter(sigma_process, sigma_measurement)}
		, m_p_kalman {create_axis_kalman_filter(sigma_process, sigma_measurement)} {};

	/*!
	 * Reset filter into a neutral state.
	 */
	void reset()
	{
		m_x_kalman.reset();
		m_y_kalman.reset();
		m_p_kalman.reset();
		m_iterations = 0;
	}

	/*!
	 * Update internal model of pen with new measurement. The state of the model can be obtained
	 * by the position, velocity, etc methods.
	 */
	void update(const Vector<f64> &position, const f64 pressure)
	{
		if (m_iterations == 0) {
			m_x_kalman.x(0, 0) = position.x();
			m_y_kalman.x(0, 0) = position.y();
			m_p_kalman.x(0, 0) = pressure;
		} else {
			m_new_x(0, 0) = position.x();
			m_x_kalman.predict();
			m_x_kalman.update(m_new_x);

			m_new_y(0, 0) = position.y();
			m_y_kalman.predict();
			m_y_kalman.update(m_new_y);

			m_new_p(0, 0) = pressure;
			m_p_kalman.predict();
			m_p_kalman.update(m_new_p);
		}

		m_iterations++;

		m_position.x() = m_x_kalman.x(0, 0);
		m_position.y() = m_y_kalman.x(0, 0);

		m_velocity.x() = m_x_kalman.x(1, 0);
		m_velocity.y() = m_y_kalman.x(1, 0);

		m_acceleration.x() = m_x_kalman.x(2, 0);
		m_acceleration.y() = m_y_kalman.x(2, 0);

		m_jank.x() = m_x_kalman.x(3, 0);
		m_jank.y() = m_y_kalman.x(3, 0);

		m_pressure = m_p_kalman.x(0, 0);
		m_pressure_change = m_p_kalman.x(1, 0);
	}

	[[nodiscard]] Vector2<f64> position() const
	{
		return m_position;
	}

	[[nodiscard]] Vector2<f64> velocity() const
	{
		return m_velocity;
	}

	[[nodiscard]] Vector2<f64> acceleration() const
	{
		return m_acceleration;
	}

	[[nodiscard]] Vector2<f64> jank() const
	{
		return m_jank;
	}

	[[nodiscard]] f64 pressure() const
	{
		return m_pressure;
	}

	[[nodiscard]] f64 pressure_change() const
	{
		return m_pressure_change;
	}

	[[nodiscard]] usize iterations() const
	{
		return m_iterations;
	}

private:
	static KalmanFilter create_axis_kalman_filter(f64 sigma_process, f64 sigma_measurement)
	{
		/*
		 * We tune the filter with a normalized dt=1, then apply the actual report rate
		 * during prediction.
		 */
		constexpr double dt = 1.0;

		KalmanFilter kalman {4, 1};

		/*
		 * State transition matrix is derived from basic physics:
		 * new_x = x + v * dt + 1/2 * a * dt^2 + 1/6 * jank * dt^3
		 * new_v = v + a * dt + 1/2 * jank * dt^2
		 * ...
		 */
		kalman.F << 1.0, dt, 0.5 * dt * dt, 0.16 * dt * dt * dt, 0.0, 1.0, dt,
			0.5 * dt * dt, 0.0, 0.0, 1.0, dt, 0, 0, 0, 1.0;

		/*
		 * We model the system noise as a noisy force on the pen.
		 * The matrix G describes the impact of that noise on each state.
		 */
		Matrix<f64> g {4, 1};
		g << 0.16 * dt * dt * dt, 0.5 * dt * dt, dt, 1;

		kalman.Q = g * g.transpose();
		kalman.Q.array() *= sigma_process;

		/*
		 * Measurements only impact the location.
		 */
		kalman.H << 1.0, 0.0, 0.0, 0.0;

		/*
		 * Measurement noise is a 1-D normal distribution.
		 */
		kalman.R(0, 0) = sigma_measurement;

		return kalman;
	}
};

} // namespace iptsd::prediction

#endif // IPTSD_PREDICTION_POINTER_KALMAN_FILTER_HPP
