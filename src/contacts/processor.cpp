// SPDX-License-Identifier: GPL-2.0-or-later

#include "processor.hpp"

#include <container/image.hpp>

#include <gsl/gsl>

namespace iptsd::contacts {

container::Image<f32> &TouchProcessor::hm()
{
	return this->tp->hm();
}

const std::vector<TouchPoint> &TouchProcessor::process()
{
	return this->tp->process();
}

const eval::perf::Registry &TouchProcessor::perf() const
{
	return this->tp->perf();
}

void TouchProcessor::resize(index2_t size)
{
	if (this->tp) {
		if (this->conf.size == size)
			return;

		this->tp.reset(nullptr);
	}

	this->conf.size = size;

	if (!this->advanced) {
		this->tp = std::make_unique<basic::TouchProcessor>(this->conf);
	} else {
		this->tp = std::make_unique<advanced::TouchProcessor>(this->conf.size);
	}

	f64 diag = std::sqrt(size.x * size.x + size.y * size.y);
	this->diag = gsl::narrow_cast<i32>(diag);
}

i32 TouchProcessor::diagonal()
{
	return this->diag;
}

} // namespace iptsd::contacts
