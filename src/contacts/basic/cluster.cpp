
// SPDX-License-Identifier: GPL-2.0-or-later

#include "cluster.hpp"

#include "heatmap.hpp"

#include <common/types.hpp>

#include <cmath>
#include <cstddef>
#include <gsl/gsl>
#include <iterator>
#include <memory>
#include <tuple>

namespace iptsd::contacts::basic {

Cluster::Cluster(Heatmap &hm, index2_t center)
{
	this->check(hm, center);
}

void Cluster::add(index2_t pos, f32 val)
{
	f32 x = gsl::narrow<f32>(pos.x);
	f32 y = gsl::narrow<f32>(pos.y);

	this->x += val * x;
	this->y += val * y;
	this->xx += val * x * x;
	this->yy += val * y * y;
	this->xy += val * x * y;
	this->w += val;

	if (this->max_v < val)
		this->max_v = val;
}

math::Vec2<f32> Cluster::mean()
{
	f32 fx = (f32)this->x;
	f32 fy = (f32)this->y;
	f32 fw = (f32)this->w;

	return math::Vec2<f32> {fx / fw, fy / fw};
}

math::Mat2s<f32> Cluster::cov()
{
	f32 r1 = (this->xx - (this->x * this->x / this->w)) / this->w;
	f32 r2 = (this->yy - (this->y * this->y / this->w)) / this->w;
	f32 r3 = (this->xy - (this->x * this->y / this->w)) / this->w;

	return math::Mat2s<f32> {r1, r3, r2};
}

void Cluster::check(Heatmap &hm, index2_t pos)
{
	f32 v = hm.value(pos);

	if (hm.get_visited(pos))
		return;

	this->add(pos, v);
	hm.set_visited(pos, true);

	this->check(hm, index2_t {pos.x + 1, pos.y});
	this->check(hm, index2_t {pos.x - 1, pos.y});
	this->check(hm, index2_t {pos.x, pos.y + 1});
	this->check(hm, index2_t {pos.x, pos.y - 1});
}

} // namespace iptsd::contacts::basic
