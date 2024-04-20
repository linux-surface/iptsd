// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_CONVOLUTION_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_CONVOLUTION_HPP

#include "optimized/convolution.3x3-extend.hpp"
#include "optimized/convolution.5x5-extend.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>

namespace iptsd::contacts::detection::convolution {

namespace impl {

/*!
 * Runs a 2D convolution of a collection and a kernel.
 *
 * This is the generic implementation for arbitrary kernel sizes.
 * Do not call this directly, use iptsd::contacts::detection::convolution::run.
 *
 * @param[in] in The input data.
 * @param[in] kernel The kernel that is applied to the input data.
 * @param[out] out A reference to the matrix where the results of the convolution are stored.
 */
template <class DerivedData, class DerivedKernel>
void run_generic(const DenseBase<DerivedData> &in,
                 const DenseBase<DerivedKernel> &kernel,
                 DenseBase<DerivedData> &out)
{
	using T = typename DenseBase<DerivedKernel>::Scalar;

	constexpr isize size_zero = 0;

	const Eigen::Index cols = in.cols();
	const Eigen::Index rows = in.rows();

	const Eigen::Index kernel_cols = kernel.cols();
	const Eigen::Index kernel_rows = kernel.rows();

	const isize dx = (casts::to_signed(kernel_cols) - 1) / 2;
	const isize dy = (casts::to_signed(kernel_rows) - 1) / 2;

	out.setZero();

	for (Eigen::Index ky = 0; ky < kernel_rows; ky++) {
		for (Eigen::Index kx = 0; kx < kernel_cols; kx++) {
			const T kern = kernel(ky, kx);

			for (Eigen::Index oy = 0; oy < rows; oy++) {
				const isize sy = casts::to_signed(oy + ky) - dy;
				const isize cy = std::clamp(sy, size_zero, rows - 1);

				const Eigen::Index iy = casts::to_eigen(cy);

				for (Eigen::Index ox = 0; ox < cols; ox++) {
					const isize sx = casts::to_signed(ox + kx) - dx;
					const isize cx = std::clamp(sx, size_zero, cols - 1);

					const Eigen::Index ix = casts::to_eigen(cx);

					out(oy, ox) += in(iy, ix) * kern;
				}
			}
		}
	}
}

} // namespace impl

/*!
 * Runs a 2D convolution of a collection and a kernel.
 *
 * If the passed kernel has a size of 3x3 or 5x5 an optimized convolution routine
 * will be used. Otherwise a generic implementation gets used.
 *
 * The borders of the input data will be extended to prevent overflowing indices.
 *
 * @param[in] in The input data.
 * @param[in] kernel The kernel that is applied to the input data.
 * @param[out] out A reference to the matrix where the results of the convolution are stored.
 */
template <class DerivedData, class DerivedKernel>
inline void run(const DenseBase<DerivedData> &in,
                const DenseBase<DerivedKernel> &kernel,
                DenseBase<DerivedData> &out)
{
	constexpr usize Rows = DerivedKernel::RowsAtCompileTime;
	constexpr usize Cols = DerivedKernel::ColsAtCompileTime;

	if constexpr (Rows == 3 && Cols == 3) {
		impl::run_3x3(in, kernel, out);
	} else if constexpr (Rows == 5 && Cols == 5) {
		impl::run_5x5(in, kernel, out);
	} else {
		if (kernel.rows() == 3 && kernel.cols() == 3)
			impl::run_3x3(in, kernel, out);
		else if (kernel.rows() == 5 && kernel.cols() == 5)
			impl::run_5x5(in, kernel, out);
		else
			impl::run_generic(in, kernel, out);
	}
}

} // namespace iptsd::contacts::detection::convolution

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_CONVOLUTION_HPP
