// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_DATA_HPP
#define IPTSD_IPTS_DATA_HPP

#include "protocol/dft.hpp"
#include "protocol/metadata.hpp"

#include <common/types.hpp>

#include <gsl/gsl>

#include <optional>

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
	u8 rows = 0;
	u8 columns = 0;

	u8 min = 0;
	u8 max = 0;

	gsl::span<u8> data {};
};

struct DftWindow {
	std::optional<u32> group = std::nullopt;
	protocol::dft::Type type {};

	u8 width = 0;
	u8 height = 0;

	gsl::span<protocol::dft::Row> x {};
	gsl::span<protocol::dft::Row> y {};
};

struct Metadata {
	protocol::metadata::Dimensions dimensions {};
	protocol::metadata::Transform transform {};
	u8 unknown_byte = 0;
	protocol::metadata::Unknown unknown {};
};

} // namespace iptsd::ipts

#endif // IPTSD_IPTS_DATA_HPP
