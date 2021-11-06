// SPDX-License-Identifier: GPL-2.0-or-later

#include "touch-manager.hpp"

#include "config.hpp"

#include <common/types.hpp>
#include <contacts/advanced/processor.hpp>
#include <contacts/basic/processor.hpp>
#include <contacts/processor.hpp>
#include <container/image.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.h>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/gsl>
#include <iterator>
#include <memory>
#include <spdlog/spdlog.h>
#include <utility>
#include <vector>

namespace iptsd::daemon {

TouchManager::TouchManager(Config conf)
	: size(), conf(conf), max_contacts(conf.info.max_contacts), inputs(max_contacts),
	  last(max_contacts), distances(max_contacts * max_contacts)
{
	for (i32 i = 0; i < this->max_contacts; i++) {
		this->last[i].index = i;
		this->last[i].active = false;
	}
}

contacts::ITouchProcessor &TouchManager::resize(u8 width, u8 height)
{
	if (this->processor) {
		if (this->size.x == width && this->size.y == height)
			return *this->processor;

		this->processor.reset(nullptr);
	}

	this->size = index2_t {width, height};

	if (true) {
		contacts::basic::TouchProcessor::Config cfg {};
		cfg.size = this->size;
		cfg.touch_thresh = this->conf.touch_threshold;

		this->processor = std::make_unique<contacts::basic::TouchProcessor>(cfg);
	} else {
		this->processor = std::make_unique<contacts::advanced::TouchProcessor>(this->size);
	}

	f64 diag = std::sqrt(width * width + height * height);
	this->diagonal = gsl::narrow_cast<i32>(diag);

	return *this->processor;
}

std::vector<TouchInput> &TouchManager::process(const ipts::Heatmap &data)
{
	contacts::ITouchProcessor &proc = this->resize(data.width, data.height);

	std::transform(data.data.begin(), data.data.end(), proc.hm().begin(), [&](auto v) {
		f32 val = static_cast<f32>(v - data.z_min) /
			  static_cast<f32>(data.z_max - data.z_min);

		return 1.0f - val;
	});

	const std::vector<contacts::TouchPoint> &contacts = proc.process();

	i32 max_contacts = this->conf.info.max_contacts;
	i32 count = std::min(gsl::narrow_cast<i32>(contacts.size()), max_contacts);

	for (i32 i = 0; i < count; i++) {
		f64 x = contacts[i].mean.x;
		f64 y = contacts[i].mean.y;

		if (this->conf.invert_x)
			x = 1 - x;

		if (this->conf.invert_y)
			y = 1 - y;

		this->inputs[i].x = gsl::narrow_cast<i32>(x * IPTS_MAX_X);
		this->inputs[i].y = gsl::narrow_cast<i32>(y * IPTS_MAX_Y);

		math::Eigen2<f32> eigen = contacts[i].cov.eigen();
		f64 s1 = std::sqrt(eigen.w[0]);
		f64 s2 = std::sqrt(eigen.w[1]);

		f64 d1 = 4 * s1 / this->diagonal;
		f64 d2 = 4 * s2 / this->diagonal;

		f64 major = std::max(d1, d2);
		f64 minor = std::min(d1, d2);

		this->inputs[i].major = gsl::narrow_cast<i32>(major * IPTS_DIAGONAL);
		this->inputs[i].minor = gsl::narrow_cast<i32>(minor * IPTS_DIAGONAL);

		math::Vec2<f64> v1 = eigen.v[0].cast<f64>() * s1;
		f64 angle = M_PI_2 - std::atan2(v1.x, v1.y);

		// Make sure that the angle is always a positive number
		if (angle < 0)
			angle += M_PI;
		if (angle > M_PI)
			angle -= M_PI;

		this->inputs[i].orientation = gsl::narrow_cast<i32>(angle / M_PI * 180);

		this->inputs[i].index = i;
		this->inputs[i].active = true;
	}

	for (i32 i = count; i < max_contacts; i++) {
		this->inputs[i].index = i;
		this->inputs[i].active = false;
	}

	this->track();

	std::swap(this->inputs, this->last);
	return this->last;
}

void TouchManager::track()
{
	// Calculate the distances between current and previous inputs
	for (u32 i = 0; i < this->max_contacts; i++) {
		for (u32 j = 0; j < this->max_contacts; j++) {
			const TouchInput &in = this->inputs[i];
			const TouchInput &last = this->last[j];

			u32 idx = i * this->max_contacts + j;

			// If one of the two inputs is / was not active, generate
			// a very high distance, so that the pair will only get chosen
			// if no "proper" pairs are left.
			if (!in.active || !last.active) {
				this->distances[idx] = (1 << 20) + idx;
				continue;
			}

			f64 dx = static_cast<f64>(in.x) - static_cast<f64>(last.x);
			f64 dy = static_cast<f64>(in.y) - static_cast<f64>(last.y);

			this->distances[idx] = std::sqrt(dx * dx + dy * dy);
		}
	}

	// Select the smallest calculated distance to find the closest two inputs.
	// Copy the index from the previous to the current input. Then invalidate
	// all distance entries that contain the two inputs, and repeat until we
	// found an index for all inputs.
	for (u32 k = 0; k < this->max_contacts; k++) {
		auto it = std::min_element(this->distances.begin(), this->distances.end());
		u32 idx = std::distance(this->distances.begin(), it);

		u32 i = idx / max_contacts;
		u32 j = idx % max_contacts;

		this->inputs[i].index = this->last[j].index;

		// Set the distance of all pairs that contain one of i and j
		// to something even higher than the distance chosen above.
		// This prevents i and j from getting selected again.
		for (u32 x = 0; x < this->max_contacts; x++) {
			u32 idx1 = i * this->max_contacts + x;
			u32 idx2 = x * this->max_contacts + j;

			this->distances[idx1] = (1 << 30) + idx1;
			this->distances[idx2] = (1 << 30) + idx2;
		}
	}
}

} // namespace iptsd::daemon
