/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_CLUSTER_HPP
#define IPTSD_CONTACTS_BASIC_CLUSTER_HPP

#include <common/types.hpp>
#include <container/image.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <vector>

namespace iptsd::contacts::basic {

class Cluster {
public:
	f64 x = 0;
	f64 y = 0;
	f64 xx = 0;
	f64 yy = 0;
	f64 xy = 0;
	f64 w = 0;

private:
	container::Image<bool> visited;

public:
	Cluster(index2_t size);

	[[nodiscard]] math::Vec2<f64> mean() const;
	[[nodiscard]] math::Mat2s<f64> cov() const;

	void add(index2_t position, f64 value);
	bool contains(index2_t position);
};

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_CLUSTER_HPP */
