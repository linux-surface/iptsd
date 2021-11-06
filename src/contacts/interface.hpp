/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_INTERFACE_HPP
#define IPTSD_CONTACTS_INTERFACE_HPP

#include "eval/perf.hpp"

#include <common/types.hpp>
#include <container/image.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <vector>

namespace iptsd::contacts {

struct TouchPoint {
	f32 confidence;
	f32 scale;
	bool palm;
	math::Vec2<f32> mean;
	math::Mat2s<f32> cov;
};

struct Config {
	index2_t size {};
	std::optional<f32> touch_thresh;
};

class ITouchProcessor {
public:
	virtual ~ITouchProcessor() = default;

	virtual container::Image<f32> &hm() = 0;
	virtual const std::vector<TouchPoint> &process() = 0;

	[[nodiscard]] virtual const eval::perf::Registry &perf() const = 0;
};

} /* namespace iptsd::contacts */

#endif /* IPTSD_CONTACTS_INTERFACE_HPP */
