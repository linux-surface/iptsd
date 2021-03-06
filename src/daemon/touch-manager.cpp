// SPDX-License-Identifier: GPL-2.0-or-later

#include "touch-manager.hpp"

#include "config.hpp"

#include <common/types.hpp>
#include <contacts/container/image.hpp>
#include <contacts/math/mat2.hpp>
#include <contacts/math/vec2.hpp>
#include <contacts/processor.hpp>
#include <contacts/types.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <gsl/gsl>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

void TouchManager::resize(u8 width, u8 height)
{
	if (this->hm) {
		iptsd::index2_t size = this->hm->size();

		if (size.x != width || size.y != height) {
			this->hm.reset(nullptr);
			this->processor.reset(nullptr);
		}
	}

	if (!this->hm) {
		iptsd::index2_t size {width, height};

		f64 diag = std::sqrt(width * width + height * height);
		this->diagonal = gsl::narrow_cast<i32>(diag);

		this->hm = std::make_unique<iptsd::container::Image<f32>>(size);
		this->processor = std::make_unique<iptsd::TouchProcessor>(size);
	}
}

std::vector<TouchInput> &TouchManager::process(const IptsHeatmap &data)
{
	this->resize(data.width, data.height);

	std::transform(data.data.begin(), data.data.end(), this->hm->begin(), [&](auto v) {
		f32 val = static_cast<f32>(v - data.z_min) /
			  static_cast<f32>(data.z_max - data.z_min);

		return 1.0f - val;
	});

	const std::vector<iptsd::TouchPoint> &contacts = this->processor->process(*this->hm);

	i32 max_contacts = this->conf.info.max_contacts;
	i32 count = std::min(gsl::narrow_cast<i32>(contacts.size()), max_contacts);

	for (i32 i = 0; i < count; i++) {
		f64 x = (contacts[i].mean.x + 0.5) / data.width;
		f64 y = (contacts[i].mean.y + 0.5) / data.height;

		if (this->conf.invert_x)
			x = 1 - x;

		if (this->conf.invert_y)
			y = 1 - y;

		this->inputs[i].x = gsl::narrow_cast<i32>(x * IPTS_MAX_X);
		this->inputs[i].y = gsl::narrow_cast<i32>(y * IPTS_MAX_Y);

		iptsd::math::Eigen2<f32> eigen = contacts[i].cov.eigen();
		f64 s1 = std::sqrt(eigen.w[0]);
		f64 s2 = std::sqrt(eigen.w[1]);

		f64 d1 = 4 * s1 / this->diagonal;
		f64 d2 = 4 * s2 / this->diagonal;

		f64 major = std::max(d1, d2);
		f64 minor = std::min(d1, d2);

		this->inputs[i].major = gsl::narrow_cast<i32>(major * IPTS_DIAGONAL);
		this->inputs[i].minor = gsl::narrow_cast<i32>(minor * IPTS_DIAGONAL);

		iptsd::math::Vec2<f64> v1 = eigen.v[0].cast<f64>() * s1;
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

	// TODO: Finger Tracking

	return this->inputs;
}
