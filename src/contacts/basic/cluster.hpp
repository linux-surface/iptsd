/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_CLUSTER_HPP
#define IPTSD_CONTACTS_BASIC_CLUSTER_HPP

#include <common/types.hpp>
#include <container/image.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

namespace iptsd::contacts::basic {

class Cluster {
public:
	f64 x = 0;
	f64 y = 0;
	f64 xx = 0;
	f64 yy = 0;
	f64 xy = 0;
	f64 w = 0;

public:
	Cluster(container::Image<f32> &heatmap, container::Image<bool> &visited, index2_t center);

	math::Vec2<f64> mean();
	math::Mat2s<f64> cov();

private:
	void add(index2_t pos, f64 val);
	void check(container::Image<f32> &heatmap, container::Image<bool> &visited, index2_t pos);
};

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_CLUSTER_HPP */
