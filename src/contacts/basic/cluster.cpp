// SPDX-License-Identifier: GPL-2.0-or-later

#include "cluster.hpp"

#include <common/types.hpp>

#include <algorithm>
#include <gsl/gsl>

namespace iptsd::contacts::basic {

Cluster::Cluster(index2_t size) : visited {size}
{
	std::fill(this->visited.begin(), this->visited.end(), false);
}

void Cluster::add(index2_t position)
{
	this->min_x = std::min(this->min_x, position.x);
	this->min_y = std::min(this->min_y, position.y);

	this->max_x = std::max(this->max_x, position.x);
	this->max_y = std::max(this->max_y, position.y);

	this->visited[position] = true;
}

void Cluster::merge(const Cluster &other)
{
	this->min_x = std::min(this->min_x, other.min_x);
	this->min_y = std::min(this->min_y, other.min_y);

	this->max_x = std::max(this->max_x, other.max_x);
	this->max_y = std::max(this->max_y, other.max_y);
}

bool Cluster::contains(index2_t position) const
{
	return this->visited[position];
}

index2_t Cluster::min() const
{
	const index_t x = std::max(this->min_x - 1, 0);
	const index_t y = std::max(this->min_y - 1, 0);

	return index2_t {x, y};
}

index2_t Cluster::max() const
{
	const index2_t size = this->visited.size();

	const index_t x = std::min(this->max_x + 1, size.x - 1);
	const index_t y = std::min(this->max_y + 1, size.y - 1);

	return index2_t {x, y};
}

index2_t Cluster::size() const
{
	// min() and max() are inclusive so we need to add one
	return (this->max() - this->min()) + index2_t {1, 1};
}

} // namespace iptsd::contacts::basic
