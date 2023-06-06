// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_KERNELS_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_KERNELS_HPP

#include <common/casts.hpp>
#include <common/types.hpp>

#include <gsl/gsl>

namespace iptsd::contacts::detection::kernels {

/*!
 * Generates a gaussian kernel.
 *
 * @tparam Rows How many rows the kernel will have.
 * @tparam Cols How many columns the kernel will have.
 * @param[in] sigma The strength of the kernel.
 * @return A gaussian kernel with the given dimensions and strength.
 */
template <class T, int Rows, int Cols>
Matrix<T, Rows, Cols> gaussian(const T sigma)
{
	static_assert(Rows % 2 == 1);
	static_assert(Cols % 2 == 1);

	T sum {};
	Matrix<T, Rows, Cols> kernel {};

	const Eigen::Index cols = kernel.cols();
	const Eigen::Index rows = kernel.rows();

	for (Eigen::Index y = 0; y < rows; y++) {
		const T vy = casts::to<T>(y) - casts::to<T>(rows - 1) / casts::to<T>(2);

		for (Eigen::Index x = 0; x < cols; x++) {
			const T vx = casts::to<T>(x) - casts::to<T>(cols - 1) / casts::to<T>(2);

			const T norm = (Vector2<T> {vy, vx} / sigma).squaredNorm();
			const T val = std::exp(gsl::narrow_cast<T>(-0.5) * norm);

			kernel(y, x) = val;
			sum += val;
		}
	}

	kernel.array() /= sum;

	return kernel;
}

} // namespace iptsd::contacts::detection::kernels

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_KERNELS_HPP
