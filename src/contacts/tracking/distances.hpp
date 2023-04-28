// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_TRACKING_DISTANCES_HPP
#define IPTSD_CONTACTS_TRACKING_DISTANCES_HPP

#include "../contact.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>

#include <limits>
#include <vector>

namespace iptsd::contacts::tracking::distances {

/*!
 * Calculates the distances between all contacts from two different frames.
 *
 * If a contact is present in one frame but not in the other, the distance will
 * be set to the highest allowed value.
 *
 * @param[in] x The first frame (x axis in the output).
 * @param[in] y The second frame (y axis in the output).
 * @param[out] out The output data.
 */
template <class Derived>
void calculate(const std::vector<Contact<typename DenseBase<Derived>::Scalar>> &x,
	       const std::vector<Contact<typename DenseBase<Derived>::Scalar>> &y,
	       DenseBase<Derived> &out)
{
	using T = typename DenseBase<Derived>::Scalar;

	const Eigen::Index sx = casts::to_eigen(x.size());
	const Eigen::Index sy = casts::to_eigen(y.size());

	out.derived().conservativeResize(sy, sx);

	// Calculate the distances between current and previous inputs
	for (Eigen::Index iy = 0; iy < sy; iy++) {
		const Contact<T> &cy = y[casts::to_unsigned(iy)];

		for (Eigen::Index ix = 0; ix < sx; ix++) {
			const Contact<T> &cx = x[casts::to_unsigned(ix)];

			out(iy, ix) = casts::to<T>((cx.mean - cy.mean).hypotNorm());
		}
	}
}

} // namespace iptsd::contacts::tracking::distances

#endif // IPTSD_CONTACTS_TRACKING_DISTANCES_HPP
