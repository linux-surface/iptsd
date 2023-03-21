// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_MAXIMAS_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_MAXIMAS_HPP

#include <common/types.hpp>

#include <vector>

namespace iptsd::contacts::detection::maximas {

/*!
 * Searches for all local maxima in the given data.
 *
 * @param[in] heatmap The data to process.
 * @param[in] threshold Only return local maxima whose value is above this threshold.
 * @param[out] maximas A reference to the vector where the found points will be stored.
 */
template <class Derived>
void find(const DenseBase<Derived> &data,
	  typename DenseBase<Derived>::Scalar threshold,
	  std::vector<Point> &maximas)
{
	using T = typename DenseBase<Derived>::Scalar;

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

	const Eigen::Index cols = data.cols();
	const Eigen::Index rows = data.rows();

	maximas.clear();

	for (Eigen::Index y = 0; y < rows; y++) {
		const bool can_up = y > 0;
		const bool can_down = y < rows - 1;

		for (Eigen::Index x = 0; x < cols; x++) {
			const T value = data(y, x);

			if (value <= threshold)
				continue;

			bool max = true;

			const bool can_left = x > 0;
			const bool can_right = x < cols - 1;

			if (can_left)
				max &= data(y, x - 1) < value;

			if (can_right)
				max &= data(y, x + 1) <= value;

			if (can_up) {
				max &= data(y - 1, x) < value;

				if (can_left)
					max &= data(y - 1, x - 1) < value;

				if (can_right)
					max &= data(y - 1, x + 1) <= value;
			}

			if (can_down) {
				max &= data(y + 1, x) <= value;

				if (can_left)
					max &= data(y + 1, x - 1) < value;

				if (can_right)
					max &= data(y + 1, x + 1) <= value;
			}

			if (max)
				maximas.emplace_back(x, y);
		}
	}
}

} // namespace iptsd::contacts::detection::maximas

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_MAXIMAS_HPP
