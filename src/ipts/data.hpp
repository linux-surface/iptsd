// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_DATA_HPP
#define IPTSD_IPTS_DATA_HPP

#include "protocol.hpp"

#include <common/types.hpp>

#include <gsl/gsl>

namespace iptsd::ipts {

struct StylusData {
	bool proximity = false;
	bool contact = false;
	bool button = false;
	bool rubber = false;

	u16 timestamp = 0;
	f64 x = 0;
	f64 y = 0;
	f64 pressure = 0;
	f64 altitude = 0;
	f64 azimuth = 0;
	u32 serial = 0;
};

struct Heatmap {
	struct ipts_dimensions dim {};
	struct ipts_timestamp time {};

	gsl::span<u8> data {};
};

struct DftWindow {
	u8 rows = 0;
	u8 type = 0;

	struct ipts_dimensions dim {};
	struct ipts_timestamp time {};

	std::array<struct ipts_pen_dft_window_row, IPTS_DFT_MAX_ROWS> x {};
	std::array<struct ipts_pen_dft_window_row, IPTS_DFT_MAX_ROWS> y {};
};

struct Metadata {
	struct ipts_touch_metadata_size size {};
	struct ipts_touch_metadata_transform transform {};
	u8 unknown_byte = 0;
	struct ipts_touch_metadata_unknown unknown {};
};

} // namespace iptsd::ipts

#endif // IPTSD_IPTS_DATA_HPP
