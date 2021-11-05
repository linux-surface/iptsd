/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_CLUSTER_HPP
#define IPTSD_CONTACTS_BASIC_CLUSTER_HPP

#include "heatmap.hpp"

#include <common/types.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <cstddef>
#include <tuple>
#include <vector>

namespace iptsd::contacts::basic {

class Cluster {
public:
	f32 x = 0;
	f32 y = 0;
	f32 xx = 0;
	f32 yy = 0;
	f32 xy = 0;
	f32 w = 0;
	f32 max_v = 0;

	Cluster(Heatmap &hm, index2_t center);

	void add(index2_t pos, f32 val);

	math::Vec2<f32> mean();
	math::Mat2s<f32> cov();

private:
	void check(Heatmap &hm, index2_t pos);
};

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_CLUSTER_HPP */
