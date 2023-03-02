// SPDX-License-Identifier: GPL-2.0-or-later

#include "algorithms.hpp"

#include "cluster.hpp"

#include <common/types.hpp>

#include <limits>

namespace iptsd::contacts::basic::algorithms {

void find_local_maximas(const container::Image<f32> &data, const f32 threshold,
			std::vector<index2_t> &out)
{
	/*
	 * We use the following kernel to compare entries:
	 *
	 *   [< ] [< ] [< ]
	 *   [< ] [  ] [<=]
	 *   [<=] [<=] [<=]
	 *
	 * Half of the entries use "less or equal", the other half "less than" as
	 * operators to ensure that we don't either discard any local maximas or
	 * report some multiple times.
	 */

	const index2_t size = data.size();

	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			const index2_t pos {x, y};

			if (data[pos] <= threshold)
				continue;

			bool max = true;
			const f32 value = data[pos];

			const bool can_up = y > 0;
			const bool can_down = y < size.y - 1;

			const bool can_left = x > 0;
			const bool can_right = x < size.x - 1;

			if (can_left)
				max &= data[{x - 1, y + 0}] < value;

			if (can_right)
				max &= data[{x + 1, y + 0}] <= value;

			if (can_up) {
				max &= data[{x + 0, y - 1}] < value;

				if (can_left)
					max &= data[{x - 1, y - 1}] < value;

				if (can_right)
					max &= data[{x + 1, y - 1}] < value;
			}

			if (can_down) {
				max &= data[{x + 0, y + 1}] <= value;

				if (can_left)
					max &= data[{x - 1, y + 1}] <= value;

				if (can_right)
					max &= data[{x + 1, y + 1}] <= value;
			}

			if (max)
				out.push_back(pos);
		}
	}
}

static void span_cluster_recursive(const container::Image<f32> &data, Cluster &cluster,
				   const f32 athresh, const f32 dthresh, const index2_t position,
				   const f32 previous)
{
	const index2_t size = data.size();

	if (position.x < 0 || position.x >= size.x)
		return;

	if (position.y < 0 || position.y >= size.y)
		return;

	const f32 value = data[position];

	if (value <= dthresh)
		return;

	// Once we left the activation area, don't allow the value to increase again
	if (previous <= athresh && value > previous)
		return;

	if (cluster.contains(position))
		return;

	cluster.add(position, value);

	span_cluster_recursive(data, cluster, athresh, dthresh, position + index2_t {1, 0}, value);
	span_cluster_recursive(data, cluster, athresh, dthresh, position + index2_t {0, 1}, value);
	span_cluster_recursive(data, cluster, athresh, dthresh, position + index2_t {-1, 0}, value);
	span_cluster_recursive(data, cluster, athresh, dthresh, position + index2_t {0, -1}, value);
}

Cluster span_cluster(const container::Image<f32> &data, const f32 athresh, const f32 dthresh,
		     const index2_t center)
{
	Cluster cluster {data.size()};

	span_cluster_recursive(data, cluster, athresh, dthresh, center,
			       std::numeric_limits<f32>::max());

	return cluster;
}

} // namespace iptsd::contacts::basic::algorithms
