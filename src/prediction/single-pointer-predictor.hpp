// SPDX-License-Identifier: Apache-2.0

#ifndef IPTSD_PREDICTION_SINGLE_POINTER_PREDICTOR_HPP
#define IPTSD_PREDICTION_SINGLE_POINTER_PREDICTOR_HPP

#include "kalman-filter.hpp"
#include "pointer-kalman-filter.hpp"

#include <common/chrono.hpp>
#include <common/types.hpp>

#include <list>
#include <numeric>

namespace iptsd::prediction {

/*!
 * Kalman based predictor, predicting the location of the pen `predictionTarget`
 * milliseconds into the future.
 *
 * This filter can provide solid prediction up to 25ms into the future. If you are not
 * achieving close-to-zero latency, prediction errors can be more visible and the target should
 * be reduced to 20ms.
 */
class SinglePointerPredictor {
private:
	using clock = chrono::steady_clock;

private:
	/*
	 * Influence of jank during each prediction sample.
	 */
	constexpr static f32 JANK_INFLUENCE = 0.1F;

	/*
	 * Influence of acceleration during each prediction sample.
	 */
	constexpr static f32 ACCELERATION_INFLUENCE = 0.5F;

	/*
	 * Influence of velocity during each prediction sample.
	 */
	constexpr static f32 VELOCITY_INFLUENCE = 1.0F;

	/*
	 * Range of jank values to expect.
	 * Low value will use maximum prediction, high value will use no prediction.
	 */
	constexpr static f32 LOW_JANK = 0.02F;
	constexpr static f32 HIGH_JANK = 0.2F;

	/*
	 * Range of pen speed to expect (in dp / ms).
	 * Low value will not use prediction, high value will use full prediction.
	 */
	constexpr static f32 LOW_SPEED = 0.0F;
	constexpr static f32 HIGH_SPEED = 2.0F;

	constexpr static auto EVENT_TIME_IGNORED_THRESHOLD = 20ms;

	/*
	 * Minimum number of Kalman filter samples needed for predicting the next point.
	 */
	constexpr static usize MIN_KALMAN_FILTER_ITERATIONS = 4;

private:
	/*
	 * The Kalman filter is tuned to smooth noise while maintaining fast reaction to direction
	 * changes. The stronger the filter, the smoother the prediction result will be, at the
	 * cost of possible prediction errors.
	 */
	PointerKalmanFilter m_kalman {0.01, 1.0};

	Vector2<f64> m_last_position {};
	clock::time_point m_prev_event_time {};
	isize m_expected_prediction_sample_size = -1;

	clock::duration m_report_rate {};
	bool m_has_fixed_report_rate = false;
	std::vector<clock::duration> m_report_rates {};

	Vector2<f64> m_position {};
	Vector2<f64> m_velocity {};
	Vector2<f64> m_acceleration {};
	Vector2<f64> m_jank {};

	f64 m_pressure = 0;

public:
	SinglePointerPredictor()
	{
		m_kalman.reset();
	}

	void reset()
	{
		m_kalman.reset();
		m_prev_event_time = clock::time_point {};
	}

	void update(const Vector2<f64> &position, const f64 pressure)
	{
		const clock::time_point event_time = clock::now();

		/*
		 * Reduce Kalman filter jank by ignoring input event with similar
		 * coordinates and eventTime as previous input event.
		 */
		if (position == m_last_position &&
		    event_time <= m_prev_event_time + EVENT_TIME_IGNORED_THRESHOLD)
			return;

		m_kalman.update(position, pressure);
		m_last_position = position;

		/*
		 * Calculate average report rate over the first 20 samples. Most sensors will not
		 * provide reliable timestamps and do not report at an even interval, so this is
		 * just to be used as an estimate.
		 */
		if (!m_has_fixed_report_rate && m_report_rates.size() < 20) {
			if (m_prev_event_time.time_since_epoch() > clock::duration::zero()) {
				m_report_rates.push_back(event_time - m_prev_event_time);

				const clock::duration sum = std::accumulate(
					m_report_rates.begin(), m_report_rates.end(),
					clock::duration::zero());

				m_report_rate = sum / m_report_rates.size();
			}
		}

		m_prev_event_time = event_time;
	}

	void set_report_rate(const clock::duration report_rate)
	{
		m_report_rate = report_rate;
		m_has_fixed_report_rate = true;
	}

	bool predict(const clock::duration prediction_target)
	{
		const f64 pt = chrono::duration_cast<milliseconds<f64>>(prediction_target).count();
		const f64 rr = chrono::duration_cast<milliseconds<f64>>(m_report_rate).count();

		if (m_has_fixed_report_rate)
			m_expected_prediction_sample_size = casts::to<isize>(std::ceil(pt / rr));

		if (m_expected_prediction_sample_size == -1 &&
		    m_kalman.iterations() < MIN_KALMAN_FILTER_ITERATIONS)
			return false;

		m_position = m_last_position;
		m_velocity = m_kalman.velocity();
		m_acceleration = m_kalman.acceleration();
		m_jank = m_kalman.jank();

		m_pressure = m_kalman.pressure();
		const f64 pressure_change = m_kalman.pressure_change();

		/*
		 * Adjust prediction distance based on confidence of kalman filter as well as
		 * movement speed.
		 */
		const f64 speed_abs = m_velocity.hypotNorm() / rr;
		const f64 speed_factor = normalize_range(speed_abs, LOW_SPEED, HIGH_SPEED);
		const f64 jank_abs = m_jank.hypotNorm();
		const f64 jank_factor = 1.0 - normalize_range(jank_abs, LOW_JANK, HIGH_JANK);
		const f64 confidence_factor = speed_factor * jank_factor;

		// Project physical state of the pen into the future.
		auto prediction_target_in_samples =
			casts::to<isize>(std::ceil(pt / rr * confidence_factor));

		// Normally this should always be false as confidenceFactor should be less than 1.0
		if (m_expected_prediction_sample_size != -1 &&
		    prediction_target_in_samples > m_expected_prediction_sample_size)
			prediction_target_in_samples = m_expected_prediction_sample_size;

		for (isize i = 0; i < prediction_target_in_samples; i++) {
			m_acceleration += m_jank * JANK_INFLUENCE;
			m_velocity += m_acceleration * ACCELERATION_INFLUENCE;
			m_position += m_velocity * VELOCITY_INFLUENCE;
			m_pressure += pressure_change;

			// Abort prediction if the pen is to be lifted.
			if (m_pressure < 0.1)
				break;

			m_pressure = std::min(m_pressure, 1.0);
		}

		return true;
	}

	[[nodiscard]] Vector2<f64> position() const
	{
		return m_position;
	}

	[[nodiscard]] f64 pressure() const
	{
		return m_pressure;
	}

private:
	static f64 normalize_range(const f64 val, const f64 min, const f64 max)
	{
		const f64 norm = (val - min) / (max - min);
		return std::clamp(norm, min, max);
	}
};

} // namespace iptsd::prediction

#endif // IPTSD_PREDICTION_SINGLE_POINTER_PREDICTOR_HPP
