// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_SUPPRESSION_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_SUPPRESSION_HPP

#include <common/casts.hpp>
#include <common/types.hpp>

#include <algorithm>
#include <utility>
#include <vector>

namespace iptsd::contacts::detection::suppression {

/*!
 * Darken pixels around each maxima by a constant factor, while leaving maxima pixels unchanged.
 *
 * This is intended to "sharpen" peaks / deepen valleys between peaks so that cluster spanning
 * is less likely to connect neighboring contacts.
 *
 * Behavior:
 *  - out is initialized as a copy of in
 *  - for each maxima, all pixels within euclidean radius (excluding the center pixel) are reduced:
 *      out(y,x) = min(out(y,x), in(y,x) * factor)
 *  - maxima pixels are restored to their original values (exactly unchanged)
 *
 * Notes:
 *  - If neighborhoods overlap, the result is still "at most factor" darkening (no stacking),
 *    because we always compare against (in * factor), not (out * factor).
 */
template <class Derived>
void darken_around_maximas(const DenseBase<Derived> &in,
                           const std::vector<Point> &maximas,
                           const Eigen::Index radius,
                           const typename DenseBase<Derived>::Scalar factor,
                           DenseBase<Derived> &out)
{
	using T = typename DenseBase<Derived>::Scalar;

	out = in;

	if (maximas.empty())
		return;

	if (radius <= 0)
		return;

	// Clamp factor to sane range. (factor = 1 => no-op, factor = 0 => nuke neighbors to 0)
	const T f = std::clamp(factor, casts::to<T>(0), casts::to<T>(1));
	if (f >= casts::to<T>(1))
		return;

	const Eigen::Index cols = in.cols();
	const Eigen::Index rows = in.rows();

	// Precompute offsets in a disk (euclidean radius).
	const isize r = casts::to_signed(radius);
	const isize r2 = r * r;

	std::vector<std::pair<isize, isize>> offsets;
	offsets.reserve(static_cast<size_t>(((2 * r) + 1) * ((2 * r) + 1)));

	for (isize dy = -r; dy <= r; ++dy) {
		for (isize dx = -r; dx <= r; ++dx) {
			if (dx == 0 && dy == 0)
				continue;

			if (((dx * dx) + (dy * dy)) <= r2)
				offsets.emplace_back(dx, dy);
		}
	}

	// Darken neighbors.
	for (const Point &p : maximas) {
		const Eigen::Index cx = p.x();
		const Eigen::Index cy = p.y();

		// Skip invalid points defensively (shouldn't happen, but costs nothing).
		if (cx < 0 || cx >= cols || cy < 0 || cy >= rows)
			continue;

		for (const auto &[dx, dy] : offsets) {
			const isize nx_s = casts::to_signed(cx) + dx;
			const isize ny_s = casts::to_signed(cy) + dy;

			if (nx_s < 0 || ny_s < 0)
				continue;

			const Eigen::Index nx = casts::to_eigen(nx_s);
			const Eigen::Index ny = casts::to_eigen(ny_s);

			if (nx >= cols || ny >= rows)
				continue;

			const T target = in(ny, nx) * f;

			// Use min() against (in*factor) to prevent multi-maxima "stacking"
			// darkness.
			out(ny, nx) = std::min(out(ny, nx), target);
		}
	}

	// Restore maxima pixels exactly.
	for (const Point &p : maximas) {
		const Eigen::Index x = p.x();
		const Eigen::Index y = p.y();

		if (x < 0 || x >= cols || y < 0 || y >= rows)
			continue;

		out(y, x) = in(y, x);
	}
}

} // namespace iptsd::contacts::detection::suppression

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_SUPPRESSION_HPP
