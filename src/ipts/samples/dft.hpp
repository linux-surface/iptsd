// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_SAMPLES_DFT_HPP
#define IPTSD_IPTS_SAMPLES_DFT_HPP

#include "../protocol/dft.hpp"

#include <common/types.hpp>

#include <gsl/gsl>

#include <optional>

namespace iptsd::ipts::samples {

struct DftWindow {
	std::optional<u32> group = std::nullopt;
	protocol::dft::Type type {};

	u8 width = 0;
	u8 height = 0;

	gsl::span<protocol::dft::Row> x {};
	gsl::span<protocol::dft::Row> y {};
};

} // namespace iptsd::ipts::samples

#endif // IPTSD_IPTS_SAMPLES_DFT_HPP
