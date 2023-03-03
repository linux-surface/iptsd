// SPDX-License-Identifier: GPL-2.0-or-later

#include "algorithms.hpp"

#include "cluster.hpp"

#include <common/types.hpp>

#include <cstddef>
#include <gsl/gsl>
#include <gsl/narrow>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

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

	cluster.add(position);

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

static f32 overlap_area(const Cluster &a, const Cluster &b)
{
	// https://stackoverflow.com/questions/25349178/calculating-percentage-of-bounding-box-overlap-for-image-detector-evaluation

	const index2_t min_a = a.min();
	const index2_t min_b = b.min();
	const index2_t max_a = a.max();
	const index2_t max_b = b.max();

	// Check if the two boxes are identical
	if (min_a.x == min_b.x && min_a.y == min_b.y && max_a.x == max_b.x && max_a.y == max_b.y)
		return 1.0f;

	// Check if the two boxes overlap
	if (min_a.x > max_b.x || min_b.x > max_a.x || min_a.y > max_b.y || min_b.y > max_a.y)
		return 0.0f;

	// Detewrminte the coordinates of the intersection rectangle
	const index_t x_left = std::max(min_a.x, min_b.x);
	const index_t y_top = std::max(min_a.y, min_b.y);
	const index_t x_right = std::min(max_a.x, max_b.x);
	const index_t y_bottom = std::min(max_a.y, max_b.y);

	if (x_right < x_left || y_bottom < y_top)
		return 0.0f;

	// The intersection of two axis-aligned bounding boxes is always an axis-aligned bounding
	// box
	const index_t intersection_area = (x_right - x_left + 1) * (y_bottom - y_top + 1);

	// Compute the area of both bounding boxes
	const index_t area_a = a.size().span();
	const index_t area_b = b.size().span();

	// Compute the intersection over union by taking the intersection area and dividing it
	// by the sum of both bounding box areas minus the intersection area
	const f32 iou = gsl::narrow<f32>(intersection_area) /
			gsl::narrow<f32>(area_a + area_b - intersection_area);

	if (iou < 0.0f || iou > 1.0f)
		throw std::runtime_error("Calculated invalid cluster overlap!");

	return iou;
}

static bool find_overlaps(const std::vector<Cluster> &clusters,
			  std::vector<std::pair<i32, i32>> &overlaps)
{
	bool found_overlap = false;
	const i32 size = gsl::narrow<i32>(clusters.size());

	for (i32 i = size - 1; i >= 0; i--) {
		const Cluster &a = clusters[i];

		// We only need to check all clusters, only the ones that weren't checked yet.
		for (i32 j = i - 1; j >= 0; j--) {
			const Cluster &b = clusters[j];

			// Ignore clusters that overlap by less than 50%
			if (overlap_area(a, b) < 0.5f)
				continue;

			found_overlap = true;

			// Because of how this look is structured, i will always be larger than j.
			// This means that if we iterate over the clusters the same way, by the
			// time we have reached j, i has already been merged and j can be dropped.
			overlaps.emplace_back(i, j);
		}
	}

	return found_overlap;
}

void merge_overlaps(std::vector<Cluster> &clusters, std::vector<Cluster> &temp, i32 iterations)
{
	std::vector<std::pair<i32, i32>> overlaps {clusters.size()};

	temp.clear();

	// Repeat the merging process until no new overlaps were detected
	for (; iterations > 0; iterations--) {
		const i32 size = gsl::narrow<i32>(clusters.size());
		const bool found_overlap = find_overlaps(clusters, overlaps);

		if (!found_overlap)
			break;

		for (i32 i = size - 1; i >= 0; i--) {
			Cluster &cluster = clusters[i];
			bool drop_cluster = false;

			for (const auto &[a, b] : overlaps) {
				// If b is the current cluster, it has already been merged
				if (b == i) {
					drop_cluster = true;
					break;
				}

				// Is this an entry for the current cluster?
				if (a != i)
					continue;

				cluster.merge(clusters[b]);
			}

			if (drop_cluster)
				continue;

			temp.push_back(std::move(cluster));
		}

		std::swap(clusters, temp);
		temp.clear();
	}

	if (iterations == 0)
		throw std::runtime_error("Failed to merge overlapping clusters!");
}

} // namespace iptsd::contacts::basic::algorithms
