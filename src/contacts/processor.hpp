/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_PROCESSOR_HPP
#define IPTSD_CONTACTS_PROCESSOR_HPP

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
	math::Vec2<f32> mean;
	math::Mat2s<f32> cov;
};

class ITouchProcessor {
public:
	virtual ~ITouchProcessor() = default;

	virtual container::Image<f32> &hm() = 0;
	[[nodiscard]] virtual const eval::perf::Registry &perf() const = 0;

	virtual const std::vector<TouchPoint> &process();
	virtual const std::vector<TouchPoint> &process(container::Image<f32> const &hm) = 0;
};

inline const std::vector<TouchPoint> &ITouchProcessor::process()
{
	return this->process(this->hm());
}

}; /* namespace iptsd::contacts */

#endif /* IPTSD_CONTACTS_PROCESSOR_HPP */
