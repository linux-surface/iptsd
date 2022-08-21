/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_INTERFACE_HPP
#define IPTSD_CONTACTS_INTERFACE_HPP

#include <common/types.hpp>
#include <container/image.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <vector>

namespace iptsd::contacts {

struct Blob {
	math::Vec2<f32> mean;
	math::Mat2s<f32> cov;
};

class IBlobDetector {
public:
	virtual ~IBlobDetector() = default;

	virtual container::Image<f32> &data() = 0;
	virtual const std::vector<Blob> &search() = 0;
};

} /* namespace iptsd::contacts */

#endif /* IPTSD_CONTACTS_INTERFACE_HPP */
