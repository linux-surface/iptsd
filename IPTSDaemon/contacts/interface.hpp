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
	math::Vec2<f64> mean;
	math::Mat2s<f64> cov;
};

enum NeutralMode {
	MODE,
	AVERAGE,
	CONSTANT,
};

struct BlobDetectorConfig {
	f32 activation_threshold;
	f32 deactivation_threshold;

	f32 neutral_value;
	enum NeutralMode neutral_mode;
};

class IBlobDetector {
public:
	virtual ~IBlobDetector() = default;

	virtual container::Image<f32> &data() = 0;
	virtual const std::vector<Blob> &search() = 0;
};

} /* namespace iptsd::contacts */

#endif /* IPTSD_CONTACTS_INTERFACE_HPP */
