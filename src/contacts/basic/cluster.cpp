// SPDX-License-Identifier: GPL-2.0-or-later

#include "cluster.hpp"

#include <common/types.hpp>

#include <gsl/gsl>

namespace iptsd::contacts::basic {

Cluster::Cluster(index2_t size) : visited {size}
{
	std::fill(this->visited.begin(), this->visited.end(), false);
}

math::Vec2<f64> Cluster::mean() const
{
	return math::Vec2<f64> {this->x / this->w, this->y / this->w};
}

math::Mat2s<f64> Cluster::cov() const
{
	const f64 r1 = (this->xx - (this->x * this->x / this->w)) / this->w;
	const f64 r2 = (this->yy - (this->y * this->y / this->w)) / this->w;
	const f64 r3 = (this->xy - (this->x * this->y / this->w)) / this->w;

	return math::Mat2s<f64> {r1, r3, r2};
}

void Cluster::add(index2_t position, f64 value)
{
	this->x += value * position.x;
	this->y += value * position.y;
	this->xx += value * position.x * position.x;
	this->yy += value * position.y * position.y;
	this->xy += value * position.x * position.y;
	this->w += value;

	this->visited[position] = true;
}

bool Cluster::contains(index2_t position)
{
	return this->visited[position];
}

} // namespace iptsd::contacts::basic
