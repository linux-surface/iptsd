// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_GENERIC_DFT_HPP
#define IPTSD_CORE_GENERIC_DFT_HPP

#include "config.hpp"

#include <common/casts.hpp>
#include <ipts/data.hpp>

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

namespace iptsd::core {

class DftStylus {
private:
	Config m_config;
	std::optional<const ipts::Metadata> m_metadata;

	// The current state of the DFT stylus.
	ipts::StylusData m_stylus;

	i32 m_real = 0;
	i32 m_imag = 0;

public:
	DftStylus(Config config, std::optional<const ipts::Metadata> metadata)
		: m_config {std::move(config)}
		, m_metadata {std::move(metadata)} {};

	/*!
	 * Loads a DFT window and calculates stylus properties from it.
	 *
	 * @param[in] dft The dft window received from the IPTS hardware.
	 */
	void input(const ipts::DftWindow &dft)
	{
		switch (dft.type) {
		case IPTS_DFT_ID_POSITION:
			this->handle_position(dft);
			break;
		case IPTS_DFT_ID_BUTTON:
			this->handle_button(dft);
			break;
		case IPTS_DFT_ID_PRESSURE:
			this->handle_pressure(dft);
			break;
		default:
			// Ignored
			break;
		}
	}

	/*!
	 * The current state of the DFT stylus.
	 *
	 * @return An object describing the current position and state of the DFT based stylus.
	 */
	[[nodiscard]] const ipts::StylusData &get_stylus() const
	{
		return m_stylus;
	}

private:
	/*!
	 * Calculates the stylus position from a DFT window.
	 *
	 * @param[in] dft The DFT window (with type == IPTS_DFT_ID_POSITION)
	 */
	void handle_position(const ipts::DftWindow &dft)
	{
		if (dft.rows <= 1) {
			this->lift();
			return;
		}

		if (dft.x[0].magnitude <= m_config.dft_position_min_mag ||
		    dft.y[0].magnitude <= m_config.dft_position_min_mag) {
			this->lift();
			return;
		}

		u8 width = dft.dim.width;
		u8 height = dft.dim.height;

		if ((width == 0 || height == 0) && m_metadata.has_value()) {
			width = casts::to<u8>(m_metadata->size.columns);
			height = casts::to<u8>(m_metadata->size.rows);
		}

		m_real = dft.x[0].real[IPTS_DFT_NUM_COMPONENTS / 2] +
			 dft.y[0].real[IPTS_DFT_NUM_COMPONENTS / 2];
		m_imag = dft.x[0].imag[IPTS_DFT_NUM_COMPONENTS / 2] +
			 dft.y[0].imag[IPTS_DFT_NUM_COMPONENTS / 2];

		f64 x = this->interpolate_position(dft.x[0]);
		f64 y = this->interpolate_position(dft.y[0]);

		if (std::isnan(x) || std::isnan(y)) {
			this->lift();
			return;
		}

		m_stylus.proximity = true;

		x /= width - 1;
		y /= height - 1;

		if (m_config.invert_x)
			x = 1 - x;

		if (m_config.invert_y)
			y = 1 - y;

		if (dft.x[1].magnitude > m_config.dft_tilt_min_mag &&
		    dft.y[1].magnitude > m_config.dft_tilt_min_mag) {
			// calculate tilt angle from relative position of secondary transmitter
			f64 xt = this->interpolate_position(dft.x[1]);
			f64 yt = this->interpolate_position(dft.y[1]);

			if (!std::isnan(xt) && !std::isnan(yt)) {
				xt /= width - 1;
				yt /= height - 1;

				if (m_config.invert_x)
					xt = 1 - xt;

				if (m_config.invert_y)
					yt = 1 - yt;

				xt -= x;
				yt -= y;

				xt *= m_config.width / m_config.dft_tilt_distance;
				yt *= m_config.height / m_config.dft_tilt_distance;

				const f64 azm = std::fmod(std::atan2(-yt, xt) + 2 * M_PI, 2 * M_PI);
				const f64 alt = std::asin(std::min(1.0, std::hypot(xt, yt)));

				m_stylus.azimuth = azm;
				m_stylus.altitude = alt;
			}
		}

		m_stylus.x = std::clamp(x, 0.0, 1.0);
		m_stylus.y = std::clamp(y, 0.0, 1.0);
	}

	/*!
	 * Calculates the button states of the stylus from a DFT window.
	 *
	 * @param[in] dft The DFT window (with type == IPTS_DFT_ID_BUTTON)
	 */
	void handle_button(const ipts::DftWindow &dft)
	{
		if (dft.rows <= 0)
			return;

		bool button = false;
		bool rubber = false;

		if (dft.x[0].magnitude > m_config.dft_button_min_mag &&
		    dft.y[0].magnitude > m_config.dft_button_min_mag) {
			const i32 real = dft.x[0].real[IPTS_DFT_NUM_COMPONENTS / 2] +
					 dft.y[0].real[IPTS_DFT_NUM_COMPONENTS / 2];
			const i32 imag = dft.x[0].imag[IPTS_DFT_NUM_COMPONENTS / 2] +
					 dft.y[0].imag[IPTS_DFT_NUM_COMPONENTS / 2];

			// same phase as position signal = eraser, opposite phase = button
			const i32 val = m_real * real + m_imag * imag;

			button = val < 0;
			rubber = val > 0;
		}

		m_stylus.button = button;
		m_stylus.rubber = rubber;
	}

	/*!
	 * Calculates the current pressure of the stylus from a DFT window.
	 *
	 * @param[in] dft The DFT window (with type == IPTS_DFT_ID_PRESSURE)
	 */
	void handle_pressure(const ipts::DftWindow &dft)
	{
		if (dft.rows < IPTS_DFT_PRESSURE_ROWS)
			return;

		f64 p = this->interpolate_frequency(dft, IPTS_DFT_PRESSURE_ROWS);
		p = 1 - p;

		if (p > 0) {
			m_stylus.contact = true;
			m_stylus.pressure = std::clamp(p, 0.0, 1.0);
		} else {
			m_stylus.contact = false;
			m_stylus.pressure = 0;
		}
	}

	/*!
	 * Interpolates the current stylus position from a list of antenna measurements.
	 *
	 * @param[in] row A list of measurements on one axis.
	 * @return The position of the stylus on that axis.
	 */
	[[nodiscard]] f64 interpolate_position(const struct ipts_pen_dft_window_row &row) const
	{
		// assume the center component has the max amplitude
		u8 maxi = IPTS_DFT_NUM_COMPONENTS / 2;

		// off-screen components are always zero, don't use them
		f64 mind = -0.5;
		f64 maxd = 0.5;

		if (gsl::at(row.real, maxi - 1) == 0 && gsl::at(row.imag, maxi - 1) == 0) {
			maxi++;
			mind = -1;
		} else if (gsl::at(row.real, maxi + 1) == 0 && gsl::at(row.imag, maxi + 1) == 0) {
			maxi--;
			maxd = 1;
		}

		// get phase-aligned amplitudes of the three center components
		const f64 amp = std::hypot(gsl::at(row.real, maxi), gsl::at(row.imag, maxi));
		if (amp < casts::to<f64>(m_config.dft_position_min_amp))
			return casts::to<f64>(NAN);

		const f64 sin = gsl::at(row.real, maxi) / amp;
		const f64 cos = gsl::at(row.imag, maxi) / amp;

		std::array<f64, 3> x = {
			sin * gsl::at(row.real, maxi - 1) + cos * gsl::at(row.imag, maxi - 1),
			amp,
			sin * gsl::at(row.real, maxi + 1) + cos * gsl::at(row.imag, maxi + 1),
		};

		// convert the amplitudes into something we can fit a parabola to
		for (u8 i = 0; i < 3; i++)
			x.at(i) = std::pow(x.at(i), m_config.dft_position_exp);

		// check orientation of fitted parabola
		if (x[0] + x[2] <= 2 * x[1])
			return casts::to<f64>(NAN);

		// find critical point of fitted parabola
		const f64 d = (x[0] - x[2]) / (2 * (x[0] - 2 * x[1] + x[2]));

		return row.first + maxi + std::clamp(d, mind, maxd);
	}

	[[nodiscard]] f64 interpolate_frequency(const ipts::DftWindow &dft, const u8 rows) const
	{
		if (rows < 3)
			return casts::to<f64>(NAN);

		// find max row
		u8 maxi = 0;
		u64 maxm = 0;

		for (u8 i = 0; i < rows; i++) {
			const u64 m = dft.x.at(i).magnitude + dft.y.at(i).magnitude;

			if (m > maxm) {
				maxm = m;
				maxi = i;
			}
		}

		if (maxm < casts::to<u64>(2 * m_config.dft_freq_min_mag))
			return casts::to<f64>(NAN);

		f64 mind = -0.5;
		f64 maxd = 0.5;

		if (maxi < 1) {
			maxi = 1;
			mind = -1;
		} else if (maxi > rows - 2) {
			maxi = rows - 2;
			maxd = 1;
		}

		/*
		 * all components in a row have the same phase, and corresponding x and y rows also
		 * have the same phase, so we can add everything together
		 */
		std::array<i32, 3> real {};
		std::array<i32, 3> imag {};

		for (u8 i = 0; i < 3; i++) {
			real.at(i) = 0;
			imag.at(i) = 0;

			for (u8 j = 0; j < IPTS_DFT_NUM_COMPONENTS; j++) {
				const struct ipts_pen_dft_window_row &x = dft.x.at(maxi + i - 1);
				const struct ipts_pen_dft_window_row &y = dft.y.at(maxi + i - 1);

				real.at(i) += gsl::at(x.real, j) + gsl::at(y.real, j);
				imag.at(i) += gsl::at(x.imag, j) + gsl::at(y.imag, j);
			}
		}

		// interpolate using Eric Jacobsen's modified quadratic estimator
		const i32 ra = real[0] - real[2];
		const i32 rb = 2 * real[1] - real[0] - real[2];
		const i32 ia = imag[0] - imag[2];
		const i32 ib = 2 * imag[1] - imag[0] - imag[2];

		const f64 d = (ra * rb + ia * ib) / casts::to<f64>(rb * rb + ib * ib);

		return (maxi + std::clamp(d, mind, maxd)) / (rows - 1);
	}

	/*!
	 * Marks the DFT stylus as lifted.
	 */
	void lift()
	{
		m_stylus.proximity = false;
		m_stylus.contact = false;
		m_stylus.button = false;
		m_stylus.rubber = false;
	}
};

} // namespace iptsd::core

#endif // IPTSD_CORE_GENERIC_DFT_HPP
