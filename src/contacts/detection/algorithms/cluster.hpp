// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_CLUSTER_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_CLUSTER_HPP

#include <common/types.hpp>

#include <limits>

namespace iptsd::contacts::detection::cluster {

namespace impl {

template <class Derived>
class RecursionState {
private:
	using T = typename DenseBase<Derived>::Scalar;

public:
	// The heatmap that is being processed.
	std::reference_wrapper<const DenseBase<Derived>> heatmap;

	T activation_threshold;
	T deactivation_threshold;

	// The cluster that is being spanned.
	std::reference_wrapper<Box> cluster;

	// Whether a point has been visited before.
	std::reference_wrapper<Image<bool>> visited;

public:
	RecursionState(const DenseBase<Derived> &heatmap,
	               const T activation_threshold,
	               const T deactivation_threshold,
	               Box &cluster,
	               Image<bool> &visited)
		: heatmap {heatmap},
		  activation_threshold {activation_threshold},
		  deactivation_threshold {deactivation_threshold},
		  cluster {cluster},
		  visited {visited} {};
};

/*!
 * Spans a cluster of points on a heatmap.
 *
 * The recursive worker function for finding clusters.
 * Do not call this directly, call @ref iptsd::contacts::detection::clusters::span()
 *
 * @param[in] state The state struct for passing constant data to each recursion step.
 * @param[in] position The pixel of the heatmap that will be checked in this step.
 * @param[in] previous The value of the pixel that was checked in the step before.
 */
template <class Derived>
void span_recursive(const RecursionState<Derived> state,
                    const Point &position,
                    const typename DenseBase<Derived>::Scalar previous)
{
	using T = typename DenseBase<Derived>::Scalar;

	const DenseBase<Derived> &heatmap = state.heatmap;

	const Eigen::Index cols = heatmap.cols();
	const Eigen::Index rows = heatmap.rows();

	const Eigen::Index x = position.x();
	const Eigen::Index y = position.y();

	const T value = heatmap(y, x);

	if (value <= state.deactivation_threshold)
		return;

	// Don't allow the value to increase outside of the activation area
	if (previous <= state.activation_threshold && value > previous)
		return;

	bool &visited = state.visited(y, x);
	Box &cluster = state.cluster;

	if (visited)
		return;

	visited = true;

	if (!cluster.contains(position))
		cluster.extend(position);

	if (x < cols - 1)
		span_recursive(state, {x + 1, y + 0}, value);

	if (x > 0)
		span_recursive(state, {x - 1, y + 0}, value);

	if (y < rows - 1)
		span_recursive(state, {x + 0, y + 1}, value);

	if (y > 0)
		span_recursive(state, {x + 0, y - 1}, value);
}

} // namespace impl

/*!
 * Spans a cluster of points on a heatmap.
 *
 * The function will begin at the starting position and recursively expand in all directions.
 * Pixels that are above the deactiviation threshold will be added to the cluster.
 * If a pixel is encountered that is below the threshold, or a pixel that has been visited
 * before, that recursion branch will terminate.
 *
 * Once the value of a pixel has fallen below the activation threshold, it is not allowed
 * to raise again, to prevent connecting two contacts into one cluster.
 *
 * @param[in] heatmap The heatmap to build a cluster from.
 * @param[in] position The starting position of the cluster (e.g. the local maxima).
 * @param[in] activation_threshold The activation threshold for searching.
 * @param[in] deactivation_threshold The deactivation threshold for searching.
 * @return The bounding box of the spanned cluster.
 */
template <class Derived>
Box span(const DenseBase<Derived> &heatmap,
         const Point &position,
         const typename DenseBase<Derived>::Scalar activation_threshold,
         const typename DenseBase<Derived>::Scalar deactivation_threshold)
{
	using T = typename DenseBase<Derived>::Scalar;

	Box cluster {};
	cluster.setEmpty();

	const Eigen::Index cols = heatmap.cols();
	const Eigen::Index rows = heatmap.rows();

	const Eigen::Index x = position.x();
	const Eigen::Index y = position.y();

	if (x < 0 || x >= cols)
		return cluster;

	if (y < 0 || y >= rows)
		return cluster;

	Image<bool> visited {rows, cols};
	visited.setConstant(false);

	const impl::RecursionState<Derived> state {
		heatmap,
		activation_threshold,
		deactivation_threshold,
		cluster,
		visited,
	};

	impl::span_recursive(state, position, std::numeric_limits<T>::max());

	return cluster;
}

} // namespace iptsd::contacts::detection::cluster

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_CLUSTER_HPP
