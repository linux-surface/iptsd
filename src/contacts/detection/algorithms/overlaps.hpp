// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_OVERLAPS_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_OVERLAPS_HPP

#include "errors.hpp"

#include <common/casts.hpp>
#include <common/error.hpp>
#include <common/types.hpp>

#include <gsl/gsl>

#include <vector>

namespace iptsd::contacts::detection::overlaps {

namespace impl {

/*!
 * Correct calculation of the area of a box.
 *
 * The built-in .volume() method is incorrect, because it is implemented as (max - min).prod().
 * min and max are inclusive, so the correct formula is (max - min + 1).prod().
 *
 * @param[in] box The box whose area will be calculated.
 * @return The area of that box.
 */
inline isize area(const Box &box)
{
	return casts::to_signed((box.sizes().array() + 1).prod());
}

/*!
 * Calculates by how many percent two clusters overlap.
 *
 * @param[in] a The first cluster.
 * @param[in] b The second cluster.
 * @return The percentage of overlap between both clusters (range 0-1).
 */
inline f64 overlap(const Box &a, const Box &b)
{
	// Check if the two boxes are identical
	if (a.isApprox(b))
		return 1.0;

	const Box intersection = a.intersection(b);

	// Check if the two boxes overlap
	if (intersection.isEmpty())
		return 0.0;

	// Compute the area of both bounding boxes
	const isize area_a = area(a);
	const isize area_b = area(b);

	const isize area_i = area(intersection);

	// Compute the intersection over union by taking the intersection area and dividing it
	// by the sum of both bounding box areas minus the intersection area
	const f64 iou = casts::to<f64>(area_i) / casts::to<f64>(area_a + area_b - area_i);

	if (iou < 0.0 || iou > 1.0)
		throw common::Error<Error::InvalidClusterOverlap> {};

	return iou;
}

/*!
 * Searches for overlaps in a list of clusters.
 *
 * @param[in] clusters The list of clusters to check for overlaps.
 * @param[out] overlaps A reference to the vector where overlapping pairs are stored.
 * @return Whether any overlaps have been found.
 */
inline bool search(const std::vector<Box> &clusters, std::vector<Vector2<usize>> &overlaps)
{
	bool found_overlap = false;
	const usize size = clusters.size();

	overlaps.clear();

	for (usize i = 0; i < size; i++) {
		const Box &a = clusters[i];

		// We only need to check all clusters, only the ones that weren't checked yet.
		for (usize j = i + 1; j < size; j++) {
			const Box &b = clusters[j];

			// Ignore clusters that overlap by less than 50%
			if (overlap(a, b) < 0.5)
				continue;

			found_overlap = true;

			// Because of how this look is structured, j will always be larger than i.
			// This means that if we iterate over the clusters the same way, by the
			// time we have reached j, i has already been merged and j can be dropped.
			overlaps.emplace_back(i, j);
		}
	}

	return found_overlap;
}

} // namespace impl

/*!
 * Merges overlapping clusters.
 *
 * The function will iterate over the list of clusters multiple times,
 * and search for ones that overlap by more than 50%. If overlaps were found,
 * one of the overlapping clusters will be extended, and the other one will be
 * dropped. If no overlaps were found in one iteration, the function returns.
 *
 * @param[in,out] clusters The list of clusters to check for overlaps.
 * @param[in] temp A temporary buffer for storing the result of an iteration.
 * @param[in] iterations How many times the function will try to merge overlaps before aborting.
 */
inline void merge(std::vector<Box> &clusters, std::vector<Box> &temp, const usize iterations)
{
	std::vector<Vector2<usize>> overlaps {clusters.size()};

	temp.clear();

	// Repeat the merging process until no new overlaps were detected
	for (usize j = 0; j < iterations; j++) {
		const usize size = clusters.size();

		if (!impl::search(clusters, overlaps))
			break;

		for (usize i = 0; i < size; i++) {
			Box &cluster = clusters[i];
			bool drop_cluster = false;

			for (const Vector2<usize> &pair : overlaps) {
				const usize a = pair.x();
				const usize b = pair.y();

				// If b is the current cluster, it has already been merged
				if (b == i) {
					drop_cluster = true;
					break;
				}

				// Is this an entry for the current cluster?
				if (a != i)
					continue;

				cluster = cluster.merged(clusters[b]);
			}

			if (drop_cluster)
				continue;

			temp.push_back(std::move(cluster));
		}

		std::swap(clusters, temp);
		temp.clear();
	}

	if (iterations == 0)
		throw common::Error<Error::FailedToMergeClusters> {};
}

} // namespace iptsd::contacts::detection::overlaps

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_OVERLAPS_HPP
