// SPDX-License-Identifier: GPL-2.0-or-later

#include "processor.hpp"

#include "cluster.hpp"

#include <common/types.hpp>
#include <contacts/interface.hpp>
#include <container/image.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <gsl/gsl>
#include <vector>

namespace iptsd::contacts::basic {

TouchProcessor::TouchProcessor(Config cfg)
	: heatmap {cfg.size}, touchpoints {}, cfg {cfg}, perfreg {}
{
	this->touchpoints.reserve(32);
}

static bool is_palm(f32 vx, f32 vy, f32 max_v)
{
	// Regular touch
	if (vx < 0.6 || (vx < 1.0 && max_v > 0.3125))
		return false;

	// Thumb
	if ((vx < 1.25 || (vx < 3.5 && max_v > 0.3515625)) && vx / vy > 1.8)
		return false;

	return true;
}

static bool is_near(const TouchPoint &a, const TouchPoint &b)
{
	math::Vec2<f32> delta = a.mean - b.mean;

	math::Eigen2<f32> eigen = b.cov.eigen();
	f32 dx = std::abs(eigen.v[0].x * delta.x + eigen.v[1].x * delta.y);
	f32 dy = std::abs(eigen.v[0].y * delta.x + eigen.v[1].y * delta.y);

	dx /= 3.2f * std::sqrt(eigen.w[0]) + 8;
	dy /= 3.2f * std::sqrt(eigen.w[1]) + 8;

	return dx * dx + dy * dy <= 1;
}

const std::vector<TouchPoint> &TouchProcessor::process()
{
	this->heatmap.reset();
	this->touchpoints.clear();

	index2_t size = this->heatmap.size;
	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			if (this->heatmap.value(pos) >= this->cfg.basic_pressure)
				continue;

			this->heatmap.set_visited(pos, true);
		}
	}

	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			if (this->heatmap.get_visited(pos))
				continue;

			Cluster cluster {this->heatmap, pos};

			math::Mat2s<f32> cov = cluster.cov();
			math::Vec2<f32> mean = cluster.mean();

			mean.x /= gsl::narrow_cast<f32>(this->heatmap.size.x) - 1.0f;
			mean.y /= gsl::narrow_cast<f32>(this->heatmap.size.y) - 1.0f;

			math::Eigen2<f32> eigen = cov.eigen();
			f32 vx = std::max(eigen.w[0], eigen.w[1]);
			f32 vy = std::min(eigen.w[0], eigen.w[1]);

			if (vy <= 0)
				continue;

			TouchPoint point {};
			point.cov = cov;
			point.mean = mean;
			point.palm = is_palm(vx, vy, cluster.max_v);
			point.confidence = 0; // TODO: Whats this?
			point.scale = 0;      // see above

			this->touchpoints.push_back(point);
		}
	}

	for (const auto &tp : this->touchpoints) {
		if (!tp.palm)
			continue;

		for (auto &o : this->touchpoints) {
			if (!is_near(tp, o))
				continue;

			o.palm = true;
		}
	}

	return this->touchpoints;
}

} // namespace iptsd::contacts::basic
