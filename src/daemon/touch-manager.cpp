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
#include <iterator>
#include <utility>
#include <vector>

TouchManager::TouchManager(IptsdConfig *conf) : inputs(conf->info.max_contacts)
{
	this->conf = conf;
	this->hm = nullptr;
	this->processor = nullptr;
}

TouchManager::~TouchManager(void)
{
	delete std::exchange(this->hm, nullptr);
	delete std::exchange(this->processor, nullptr);
}

void TouchManager::resize(u8 width, u8 height)
{
	if (this->hm) {
		index2_t size = this->hm->size();

		if (size.x != width || size.y != height) {
			delete std::exchange(this->hm, nullptr);
			delete std::exchange(this->processor, nullptr);
		}
	}

	if (!this->hm) {
		index2_t size;
		size.x = width;
		size.y = height;

		this->hm = new container::image<f32>(size);
		this->processor = new touch_processor(size);
	}
}

std::vector<TouchInput> &TouchManager::process(IptsHeatmap data)
{
	this->resize(data.width, data.height);

	std::transform(data.data.begin(), data.data.end(), this->hm->begin(), [&](auto v) {
		return 1.0f - (f32)(v - data.z_min) / (f32)(data.z_max - data.z_min);
	});

	std::vector<touch_point> contacts = this->processor->process(*this->hm);

	size_t count = std::min(std::size(contacts), (size_t)this->conf->info.max_contacts);
	i32 diag = std::sqrt(data.width * data.width + data.height + data.height);

	for (size_t i = 0; i < count; i++) {
		f64 x = (contacts[i].mean.x + 0.5) / data.width;
		f64 y = (contacts[i].mean.y + 0.5) / data.height;

		if (this->conf->invert_x)
			x = 1 - x;

		if (this->conf->invert_y)
			y = 1 - y;

		this->inputs[i].x = (i32)(x * IPTS_MAX_X);
		this->inputs[i].y = (i32)(y * IPTS_MAX_Y);

		math::eigen2_t<f32> eigen = contacts[i].cov.eigen();
		f64 s1 = std::sqrt(eigen.w[0]);
		f64 s2 = std::sqrt(eigen.w[1]);

		f32 d1 = 4 * s1 / diag;
		f32 d2 = 4 * s2 / diag;

		f32 major = std::max(d1, d2);
		f32 minor = std::min(d1, d2);

		this->inputs[i].major = (i32)(major * IPTS_DIAGONAL);
		this->inputs[i].minor = (i32)(minor * IPTS_DIAGONAL);

		math::vec2_t<f64> v1 = eigen.v[0].cast<f64>() * s1;
		f64 angle = M_PI_2 - std::atan2(v1.x, v1.y);

		// Make sure that the angle is always a positive number
		if (angle < 0)
			angle += M_PI;
		if (angle > M_PI)
			angle -= M_PI;

		this->inputs[i].orientation = (i32)(angle / M_PI * 180);

		this->inputs[i].index = i;
		this->inputs[i].slot = i;
	}

	for (size_t i = count; i < std::size(this->inputs); i++) {
		this->inputs[i].x = 0;
		this->inputs[i].y = 0;
		this->inputs[i].orientation = 0;
		this->inputs[i].major = 0;
		this->inputs[i].minor = 0;
		this->inputs[i].slot = i;
		this->inputs[i].index = -1;
	}

	// TODO: Finger Tracking

	return this->inputs;
}
