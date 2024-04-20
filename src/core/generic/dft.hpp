// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_GENERIC_DFT_HPP
#define IPTSD_CORE_GENERIC_DFT_HPP

#include "config.hpp"
#include "device.hpp"

#include <common/casts.hpp>
#include <ipts/metadata.hpp>
#include <ipts/protocol/dft.hpp>
#include <ipts/samples/dft.hpp>
#include <ipts/samples/stylus.hpp>

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

namespace iptsd::core {

class DftStylus {
private:
	Config m_config;
	DeviceInfo m_info;

	// The current state of the DFT stylus.
	ipts::samples::Stylus m_stylus;

	i32 m_real = 0;
	i32 m_imag = 0;
	std::optional<u32> m_group = std::nullopt;

	// For the MPP v2 button detection we only care about the first binary
	// (0x0a) dft window, but there's two in the group, we keep track of the
	// group when 0x0a was encountered, this allows comparing against this
	// value to use only the first window in the group.
	std::optional<u32> m_mppv2_binary_group = std::nullopt;

	// Boolean to track whether a button is held according to mpp v2.
	// This is used instead of the button threshold detection.
	std::optional<bool> m_mppv2_button_or_eraser = std::nullopt;

	// Boolean to track whether the pen is in contact according to mpp v2.
	// This is used to override the contact state in the pressure frame handling.
	std::optional<bool> m_mppv2_in_contact = std::nullopt;

public:
	DftStylus(Config config, const DeviceInfo &info)
		: m_config {std::move(config)},
		  m_info {info} {};

	/*!
	 * Loads a DFT window and calculates stylus properties from it.
	 *
	 * @param[in] dft The dft window received from the IPTS hardware.
	 */
	void input(const ipts::samples::DftWindow &dft)
	{
		switch (dft.type) {
		case ipts::protocol::dft::Type::Position:
			this->handle_position(dft);
			break;
		case ipts::protocol::dft::Type::Button:
			this->handle_button(dft);
			break;
		case ipts::protocol::dft::Type::Pressure:
			this->handle_pressure(dft);
			break;
		case ipts::protocol::dft::Type::PositionMPP_2:
			this->handle_position_mpp_2(dft);
			break;
		case ipts::protocol::dft::Type::BinaryMPP_2:
			this->handle_dft_binary_mpp_2(dft);
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
	[[nodiscard]] const ipts::samples::Stylus &get_stylus() const
	{
		return m_stylus;
	}

private:
	/*!
	 * Calculates the stylus position from a DFT window.
	 *
	 * @param[in] dft The DFT window (with type == Type::Position)
	 */
	void handle_position(const ipts::samples::DftWindow &dft)
	{
		if (dft.x.size() <= 1) {
			this->lift();
			return;
		}

		if (dft.x[0].magnitude <= m_config.dft_position_min_mag ||
		    dft.y[0].magnitude <= m_config.dft_position_min_mag) {
			this->lift();
			return;
		}

		u8 width = dft.width;
		u8 height = dft.height;

		if ((width == 0 || height == 0) && m_info.meta.has_value()) {
			width = m_info.meta->columns;
			height = m_info.meta->rows;
		}

		m_group = dft.group;

		m_real = dft.x[0].real[ipts::protocol::dft::NUM_COMPONENTS / 2] +
		         dft.y[0].real[ipts::protocol::dft::NUM_COMPONENTS / 2];
		m_imag = dft.x[0].imag[ipts::protocol::dft::NUM_COMPONENTS / 2] +
		         dft.y[0].imag[ipts::protocol::dft::NUM_COMPONENTS / 2];

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
	 * @param[in] dft The DFT window (with type == Type::Button)
	 */
	void handle_button(const ipts::samples::DftWindow &dft)
	{
		if (dft.x.empty())
			return;

		// The position and button signals must be from the same group,
		// otherwise the relative phase is meaningless.
		if (!m_group.has_value() || m_group != dft.group)
			return;

		bool button = false;
		bool rubber = false;

		// If mppv2 has decided on a button state use that, else use the magnitude decision.
		if (m_mppv2_button_or_eraser.value_or(
			    dft.x[0].magnitude > m_config.dft_button_min_mag &&
			    dft.y[0].magnitude > m_config.dft_button_min_mag)) {
			const i32 real = dft.x[0].real[ipts::protocol::dft::NUM_COMPONENTS / 2] +
			                 dft.y[0].real[ipts::protocol::dft::NUM_COMPONENTS / 2];
			const i32 imag = dft.x[0].imag[ipts::protocol::dft::NUM_COMPONENTS / 2] +
			                 dft.y[0].imag[ipts::protocol::dft::NUM_COMPONENTS / 2];

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
	 * @param[in] dft The DFT window (with type == Type::Pressure)
	 */
	void handle_pressure(const ipts::samples::DftWindow &dft)
	{
		if (dft.x.size() < ipts::protocol::dft::PRESSURE_ROWS)
			return;

		const f64 p =
			1 - this->interpolate_frequency(dft, ipts::protocol::dft::PRESSURE_ROWS);

		if (p > 0) {
			m_stylus.contact = true;
			m_stylus.pressure = std::clamp(p, 0.0, 1.0);
		} else {
			m_stylus.contact = m_mppv2_in_contact.value_or(false);
			m_stylus.pressure = 0;
		}
	}

	/*!
	 * Determines the current button state from the 0x0a frame, it can
	 * only be used for MPP v2 pens. The eraser is still obtained from the
	 * phase using the button frame.
	 */
	void handle_dft_binary_mpp_2(const ipts::samples::DftWindow &dft)
	{
		if (dft.x.size() <= 5) { // not sure if this can happen?
			return;
		}

		// Second time we see this dft window in this group, skip it.
		if (!dft.group.has_value() || m_mppv2_binary_group == dft.group)
			return;
		m_mppv2_binary_group = dft.group;

		// Clearing the state in case we can't determine it.
		m_mppv2_button_or_eraser = std::nullopt;

		// Now, we can process the frame to determine button state.
		// First, collapse x and y, they convey the same information.
		const auto mag_4 = dft.x[4].magnitude + dft.y[4].magnitude;
		const auto mag_5 = dft.x[5].magnitude + dft.y[5].magnitude;
		const auto threshold = 2 * m_config.dft_mpp2_button_min_mag;

		if (mag_4 < threshold && mag_5 < threshold) {
			// Not enough signal to make a decision.
			return;
		}

		// One of them is above the threshold, if 5 is higher than 4, button
		// is held.
		m_mppv2_button_or_eraser = mag_4 < mag_5;
	}

	/*!
	 * Determines whether the pen is making contact with the screen, it can
	 * only be used for MPP v2 pens.
	 */
	void handle_position_mpp_2(const ipts::samples::DftWindow &dft)
	{
		// Clearing the state in case we can't determine it.
		m_mppv2_in_contact = std::nullopt;

		if (dft.x.size() <= 3) { // not sure if this can happen?
			return;
		}

		const auto mag_2 = dft.x[2].magnitude + dft.y[2].magnitude;
		const auto mag_3 = dft.x[3].magnitude + dft.y[3].magnitude;

		const auto threshold = 2 * m_config.dft_mpp2_contact_min_mag;
		if (mag_2 < threshold && mag_3 < threshold) {
			// Not enough signal to make a decision.
			return;
		}

		// The pen switches the row from two to three when there's contact.
		m_mppv2_in_contact = mag_2 < mag_3;
	}

	/*!
	 * Interpolates the current stylus position from a list of antenna measurements.
	 *
	 * @param[in] row A list of measurements on one axis.
	 * @return The position of the stylus on that axis.
	 */
	[[nodiscard]] f64 interpolate_position(const ipts::protocol::dft::Row &row) const
	{
		// assume the center component has the max amplitude
		u8 maxi = ipts::protocol::dft::NUM_COMPONENTS / 2;

		// off-screen components are always zero, don't use them
		f64 mind = -0.5;
		f64 maxd = 0.5;

		if (row.real.at(maxi - 1) == 0 && row.imag.at(maxi - 1) == 0) {
			maxi++;
			mind = -1;
		} else if (row.real.at(maxi + 1) == 0 && row.imag.at(maxi + 1) == 0) {
			maxi--;
			maxd = 1;
		}

		// get phase-aligned amplitudes of the three center components
		const f64 amp = std::hypot(row.real.at(maxi), row.imag.at(maxi));

		if (amp < casts::to<f64>(m_config.dft_position_min_amp))
			return casts::to<f64>(NAN);

		const f64 sin = row.real.at(maxi) / amp;
		const f64 cos = row.imag.at(maxi) / amp;

		std::array<f64, 3> x = {
			sin * row.real.at(maxi - 1) + cos * row.imag.at(maxi - 1),
			amp,
			sin * row.real.at(maxi + 1) + cos * row.imag.at(maxi + 1),
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

	[[nodiscard]] f64 interpolate_frequency(const ipts::samples::DftWindow &dft,
	                                        const u8 rows) const
	{
		if (rows < 3)
			return casts::to<f64>(NAN);

		// find max row
		u8 maxi = 0;
		u64 maxm = 0;

		for (u8 i = 0; i < rows; i++) {
			const u64 m = dft.x[i].magnitude + dft.y[i].magnitude;

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

			for (u8 j = 0; j < ipts::protocol::dft::NUM_COMPONENTS; j++) {
				const ipts::protocol::dft::Row &x = dft.x[maxi + i - 1];
				const ipts::protocol::dft::Row &y = dft.y[maxi + i - 1];

				real.at(i) += x.real.at(j) + y.real.at(j);
				imag.at(i) += x.imag.at(j) + y.imag.at(j);
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

		m_mppv2_in_contact = std::nullopt;
		m_mppv2_button_or_eraser = std::nullopt;
	}
};

} // namespace iptsd::core

#endif // IPTSD_CORE_GENERIC_DFT_HPP
