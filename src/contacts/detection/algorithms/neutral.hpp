// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_NEUTRAL_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_NEUTRAL_HPP

#include "errors.hpp"

#include <common/error.hpp>
#include <common/types.hpp>

namespace iptsd::contacts::detection::neutral {

namespace impl {

/*!
 * Calculates the statistical mode of a data set.
 *
 * @param[in] data: The input data set.
 * @return The statistical mode of all values in the data set.
 */
template <class Derived>
typename DenseBase<Derived>::Scalar statistical_mode(const DenseBase<Derived> &data)
{
	using T = typename DenseBase<Derived>::Scalar;

	const Eigen::Index cols = data.cols();
	const Eigen::Index rows = data.rows();

	std::map<T, u32> counts {};

	u32 max_count = 0;
	T max_element {};

	for (Eigen::Index y = 0; y < rows; y++) {
		for (Eigen::Index x = 0; x < cols; x++) {
			const T value = data(y, x);
			const u32 count = ++counts[value];

			if (count > max_count) {
				max_count = count;
				max_element = value;
			}
		}
	}

	return max_element;
}

} // namespace impl

/*
 * The algorithm that will be used to calculate the neutral value.
 */
enum class Algorithm : u8 {
	// The most common element (statistical mode) will be used.
	MODE,

	// The average of all elements will be used.
	AVERAGE,

	// A constant value will be used.
	CONSTANT,
};

/*!
 * Calculates the neutral value of a heatmap.
 *
 * The neutral value is the value that marks an area that has no contacts.
 * Everything below this value can be safely ignored as noise.
 *
 * @param[in] heatmap: The input heatmap.
 * @param[in] algorithm: The algorithm to use for calculating the neutral value.
 * @param[in] offset: The offset to add to the calculated value.
 * @return The neutral value of all values in the heatmap.
 */
template <class Derived>
typename DenseBase<Derived>::Scalar calculate(const DenseBase<Derived> &heatmap,
                                              const Algorithm algorithm,
                                              const typename DenseBase<Derived>::Scalar offset)
{
	switch (algorithm) {
	case Algorithm::MODE:
		return impl::statistical_mode(heatmap) + offset;
	case Algorithm::AVERAGE:
		return heatmap.mean() + offset;
	case Algorithm::CONSTANT:
		return offset;
	default:
		throw common::Error<Error::InvalidNeutralMode> {};
	}
}

} // namespace iptsd::contacts::detection::neutral

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_NEUTRAL_HPP
