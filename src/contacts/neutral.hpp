/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_NEUTRAL_HPP
#define IPTSD_CONTACTS_NEUTRAL_HPP

#include "interface.hpp"

#include <common/types.hpp>
#include <container/image.hpp>
#include <container/ops.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <gsl/gsl>
#include <vector>

namespace iptsd::contacts {

inline f32 neutral_mode(const container::Image<f32> &data)
{
	index2_t size = data.size();
	std::array<u32, UINT8_MAX + 1> count {};

	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			u8 val = gsl::narrow_cast<u8>(data[pos] * UINT8_MAX);
			count.at(val)++;
		}
	}

	auto max = std::max_element(count.begin(), count.end());
	u32 idx = std::distance(count.begin(), max);

	return gsl::narrow<f32>(idx + 1) / UINT8_MAX;
}

inline f32 neutral_average(const container::Image<f32> &data)
{
	f32 sum = container::ops::sum(data);
	return sum / gsl::narrow<f32>(data.size().span());
}

inline f32 neutral(BlobDetectorConfig config, const container::Image<f32> &data)
{
	switch (config.neutral_mode) {
	case NeutralMode::MODE:
		return neutral_mode(data);
	case NeutralMode::AVERAGE:
		return neutral_average(data);
	case NeutralMode::CONSTANT:
		return config.neutral_value / 255;
	default:
		throw std::runtime_error("Invalid threshold_mode!");
	}
}

} /* namespace iptsd::contacts */

#endif /* IPTSD_CONTACTS_NEUTRAL_HPP */
