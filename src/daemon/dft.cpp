// SPDX-License-Identifier: GPL-2.0-or-later

#include "dft.hpp"

#include <ipts/parser.hpp>
#include <ipts/protocol.hpp>

#include <algorithm>
#include <cmath>
#include <gsl/gsl>
#include <tuple>

namespace iptsd::daemon {

static void iptsd_dft_lift(ipts::StylusData &stylus)
{
	stylus.proximity = false;
	stylus.rubber = false;
	stylus.contact = false;
	stylus.button = false;
}

static std::tuple<bool, f64>
iptsd_dft_interpolate_position(const Context &ctx, const struct ipts_pen_dft_window_row &row)
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
	f64 amp = std::hypot(gsl::at(row.real, maxi), gsl::at(row.imag, maxi));
	if (amp < ctx.config.dft_position_min_amp)
		return std::tuple {false, 0};

	f64 sin = gsl::at(row.real, maxi) / amp;
	f64 cos = gsl::at(row.imag, maxi) / amp;

	std::array<f64, 3> x = {
		sin * gsl::at(row.real, maxi - 1) + cos * gsl::at(row.imag, maxi - 1),
		amp,
		sin * gsl::at(row.real, maxi + 1) + cos * gsl::at(row.imag, maxi + 1),
	};

	// convert the amplitudes into something we can fit a parabola to
	for (u8 i = 0; i < 3; i++)
		x.at(i) = std::pow(x.at(i), ctx.config.dft_position_exp);

	// check orientation of fitted parabola
	if (x[0] + x[2] <= 2 * x[1])
		return std::tuple {false, 0};

	// find critical point of fitted parabola
	f64 d = (x[0] - x[2]) / (2 * (x[0] - 2 * x[1] + x[2]));

	if (std::isnan(d))
		return std::tuple {false, 0};

	return std::tuple {true, row.first + maxi + std::clamp(d, mind, maxd)};
}

static f64 iptsd_dft_interpolate_frequency(const Context &ctx, const ipts::DftWindow &dft, u8 rows)
{
	if (rows < 3)
		return NAN;

	// find max row
	u8 maxi = 0;
	u64 maxm = 0;

	for (u8 i = 0; i < rows; i++) {
		u64 m = dft.x.at(i).magnitude + dft.y.at(i).magnitude;
		if (m > maxm) {
			maxm = m;
			maxi = i;
		}
	}

	if (maxm < static_cast<u64>(2 * ctx.config.dft_freq_min_mag))
		return NAN;

	f64 mind = -0.5;
	f64 maxd = 0.5;

	if (maxi < 1) {
		maxi = 1;
		mind = -1;
	} else if (maxi > rows - 2) {
		maxi = rows - 2;
		maxd = 1;
	}

	// all components in a row have the same phase, and corresponding x and y rows also have the
	// same phase, so we can add everything together
	std::array<i32, 3> real {};
	std::array<i32, 3> imag {};

	for (u8 i = 0; i < 3; i++) {
		real.at(i) = 0;
		imag.at(i) = 0;

		for (u8 j = 0; j < IPTS_DFT_NUM_COMPONENTS; j++) {
			const auto &x = dft.x.at(maxi + i - 1);
			const auto &y = dft.y.at(maxi + i - 1);

			real.at(i) += gsl::at(x.real, j) + gsl::at(y.real, j);
			imag.at(i) += gsl::at(x.imag, j) + gsl::at(y.imag, j);
		}
	}

	// interpolate using Eric Jacobsen's modified quadratic estimator
	i32 ra = real[0] - real[2];
	i32 rb = 2 * real[1] - real[0] - real[2];
	i32 ia = imag[0] - imag[2];
	i32 ib = 2 * imag[1] - imag[0] - imag[2];

	f64 d = (ra * rb + ia * ib) / static_cast<f64>(rb * rb + ib * ib);

	return (maxi + std::clamp(d, mind, maxd)) / (rows - 1);
}

static void iptsd_dft_handle_position(Context &ctx, const ipts::DftWindow &dft,
				      ipts::StylusData &stylus)
{
	if (dft.rows <= 1) {
		iptsd_dft_lift(stylus);
		return;
	}

	if (dft.x[0].magnitude <= ctx.config.dft_position_min_mag ||
	    dft.y[0].magnitude <= ctx.config.dft_position_min_mag) {
		iptsd_dft_lift(stylus);
		return;
	}

	stylus.real = dft.x[0].real[IPTS_DFT_NUM_COMPONENTS / 2] +
		      dft.y[0].real[IPTS_DFT_NUM_COMPONENTS / 2];
	stylus.imag = dft.x[0].imag[IPTS_DFT_NUM_COMPONENTS / 2] +
		      dft.y[0].imag[IPTS_DFT_NUM_COMPONENTS / 2];

	auto [px, x] = iptsd_dft_interpolate_position(ctx, dft.x[0]);
	auto [py, y] = iptsd_dft_interpolate_position(ctx, dft.y[0]);

	if (px && py) {
		stylus.proximity = true;

		x /= dft.dim.width - 1;
		y /= dft.dim.height - 1;

		if (ctx.config.invert_x)
			x = 1 - x;

		if (ctx.config.invert_y)
			y = 1 - y;

		if (dft.x[1].magnitude > ctx.config.dft_tilt_min_mag &&
		    dft.y[1].magnitude > ctx.config.dft_tilt_min_mag) {
			// calculate tilt angle from relative position of secondary transmitter

			auto [pxt, xt] = iptsd_dft_interpolate_position(ctx, dft.x[1]);
			auto [pyt, yt] = iptsd_dft_interpolate_position(ctx, dft.y[1]);

			if (pxt && pyt) {
				xt /= dft.dim.width - 1;
				yt /= dft.dim.height - 1;

				if (ctx.config.invert_x)
					xt = 1 - xt;

				if (ctx.config.invert_y)
					yt = 1 - yt;

				xt -= x;
				yt -= y;

				if (ctx.config.dft_tip_distance) {
					// correct tip position using tilt data
					auto r = ctx.config.dft_tip_distance /
						 ctx.config.dft_tilt_distance;
					x -= xt * r;
					y -= yt * r;
				}

				xt *= ctx.config.width / ctx.config.dft_tilt_distance;
				yt *= ctx.config.height / ctx.config.dft_tilt_distance;

				auto azm = std::fmod(std::atan2(-yt, xt) / M_PI + 2, 2) * 18000;
				auto alt =
					std::asin(std::min(1.0, std::hypot(xt, yt))) / M_PI * 18000;
				stylus.azimuth = gsl::narrow<u16>(std::round(azm));
				stylus.altitude = gsl::narrow<u16>(std::round(alt));
			}
		}

		x = std::round(std::clamp(x, 0.0, 1.0) * IPTS_MAX_X);
		y = std::round(std::clamp(y, 0.0, 1.0) * IPTS_MAX_Y);

		stylus.x = gsl::narrow_cast<u16>(x);
		stylus.y = gsl::narrow_cast<u16>(y);
	} else {
		iptsd_dft_lift(stylus);
	}
}

static void iptsd_dft_handle_button(Context &ctx, const ipts::DftWindow &dft,
				    ipts::StylusData &stylus)
{
	if (dft.rows <= 0)
		return;

	bool button = false;
	bool rubber = false;

	if (dft.x[0].magnitude > ctx.config.dft_button_min_mag &&
	    dft.y[0].magnitude > ctx.config.dft_button_min_mag) {
		i32 real = dft.x[0].real[IPTS_DFT_NUM_COMPONENTS / 2] +
			   dft.y[0].real[IPTS_DFT_NUM_COMPONENTS / 2];
		i32 imag = dft.x[0].imag[IPTS_DFT_NUM_COMPONENTS / 2] +
			   dft.y[0].imag[IPTS_DFT_NUM_COMPONENTS / 2];

		// same phase as position signal = eraser, opposite phase = button
		i32 val = stylus.real * real + stylus.imag * imag;

		button = val < 0;
		rubber = val > 0;
	}

	// toggling rubber while proximity is true seems to cause issues, so set proximity off first
	if (stylus.rubber != rubber)
		iptsd_dft_lift(stylus);

	stylus.button = button;
	stylus.rubber = rubber;
}

static void iptsd_dft_handle_pressure(Context &ctx, const ipts::DftWindow &dft,
				      ipts::StylusData &stylus)
{
	if (dft.rows < IPTS_DFT_PRESSURE_ROWS)
		return;

	f64 p = iptsd_dft_interpolate_frequency(ctx, dft, IPTS_DFT_PRESSURE_ROWS);
	p = (1 - p) * IPTS_MAX_PRESSURE;

	if (p > 1) {
		stylus.contact = true;
		stylus.pressure = std::min(gsl::narrow_cast<u16>(p), IPTS_MAX_PRESSURE);
	} else {
		stylus.contact = false;
		stylus.pressure = 0;
	}
}

void iptsd_dft_input(Context &ctx, const ipts::DftWindow &dft, ipts::StylusData &stylus)
{
	switch (dft.type) {
	case IPTS_DFT_ID_POSITION:
		iptsd_dft_handle_position(ctx, dft, stylus);
		break;
	case IPTS_DFT_ID_BUTTON:
		iptsd_dft_handle_button(ctx, dft, stylus);
		break;
	case IPTS_DFT_ID_PRESSURE:
		iptsd_dft_handle_pressure(ctx, dft, stylus);
		break;
	}
}

} // namespace iptsd::daemon
