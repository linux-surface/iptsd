// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_KERNELS_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_KERNELS_HPP

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
		const f64 dy = gsl::narrow<f64>(y) - (rows - 1) / 2.0;
		const T vy = gsl::narrow_cast<T>(dy);

		for (Eigen::Index x = 0; x < cols; x++) {
			const f64 dx = gsl::narrow<f64>(x) - (cols - 1) / 2.0;
			const T vx = gsl::narrow_cast<T>(dx);

			const T norm = (Vector2<T> {vy, vx} / sigma).squaredNorm();
			const T val = std::exp(-0.5F * norm);

			kernel(y, x) = val;
			sum += val;
		}
	}

	kernel.array() /= sum;

	return kernel;
}

} // namespace iptsd::contacts::detection::kernels

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_KERNELS_HPP
