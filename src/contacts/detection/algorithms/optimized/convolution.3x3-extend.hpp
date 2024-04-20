// SPDX-License-Identifier: GPL-2.0-or-later

#include <common/buildopts.hpp>
#include <common/casts.hpp>
#include <common/types.hpp>

namespace iptsd::contacts::detection::convolution::impl {

/*!
 * Runs a 2D convolution of a matrix and a kernel.
 *
 * This is the optimized implementation for 3x3 kernels.
 * Do not call this directly, use @ref iptsd::contacts::detection::convolution::run().
 *
 * @param[in] in The input data.
 * @param[in] kernel The kernel that is applied to the input data.
 * @param[out] out A reference to the matrix where the results of the convolution are stored.
 */
template <class DerivedData, class DerivedKernel>
inline void run_3x3(const DenseBase<DerivedData> &in,
                    const DenseBase<DerivedKernel> &kernel,
                    DenseBase<DerivedData> &out)
{
	using T = typename DenseBase<DerivedData>::Scalar;
	using S = typename DenseBase<DerivedKernel>::Scalar;

	const Eigen::Index cols = in.cols();
	const Eigen::Index rows = in.rows();

	// access helpers
	const auto k = [&](isize dx, isize dy) constexpr -> S {
		const Eigen::Index x = casts::to_eigen(dx + 1);
		const Eigen::Index y = casts::to_eigen(dy + 1);

		if constexpr (common::buildopts::ForceAccessChecks) {
			return kernel(y, x);
		} else {
			return kernel.coeff(y, x);
		}
	};

	const auto d = [&](Eigen::Index i, isize dx, isize dy) constexpr -> T {
		const isize sdx = casts::to_signed(i) + dy * casts::to_signed(cols) + dx;
		const Eigen::Index index = casts::to_eigen(sdx);

		if constexpr (common::buildopts::ForceAccessChecks) {
			return in(index);
		} else {
			return in.coeff(index);
		}
	};

	// processing...
	Eigen::Index i = 0;

	// y = 0
	{
		// x = 0
		{
			auto v = casts::to<T>(0);

			v += d(i, 0, 0) * k(-1, -1); // extended
			v += d(i, 0, 0) * k(0, -1);  // extended
			v += d(i, 1, 0) * k(1, -1);  // extended

			v += d(i, 0, 0) * k(-1, 0); // extended
			v += d(i, 0, 0) * k(0, 0);
			v += d(i, 1, 0) * k(1, 0);

			v += d(i, 0, 1) * k(-1, 1); // extended
			v += d(i, 0, 1) * k(0, 1);
			v += d(i, 1, 1) * k(1, 1);

			out(i++) = v;
		}

		// 0 < x < n
		const auto limit = i + cols - 2;
		while (i < limit) {
			auto v = casts::to<T>(0);

			v += d(i, -1, 0) * k(-1, -1); // extended
			v += d(i, 0, 0) * k(0, -1);   // extended
			v += d(i, 1, 0) * k(1, -1);   // extended

			v += d(i, -1, 0) * k(-1, 0);
			v += d(i, 0, 0) * k(0, 0);
			v += d(i, 1, 0) * k(1, 0);

			v += d(i, -1, 1) * k(-1, 1);
			v += d(i, 0, 1) * k(0, 1);
			v += d(i, 1, 1) * k(1, 1);

			out(i++) = v;
		}

		// x = n - 1
		{
			auto v = casts::to<T>(0);

			v += d(i, -1, 0) * k(-1, -1); // extended
			v += d(i, 0, 0) * k(0, -1);   // extended
			v += d(i, 0, 0) * k(1, -1);   // extended

			v += d(i, -1, 0) * k(-1, 0);
			v += d(i, 0, 0) * k(0, 0);
			v += d(i, 0, 0) * k(1, 0); // extended

			v += d(i, -1, 1) * k(-1, 1);
			v += d(i, 0, 1) * k(0, 1);
			v += d(i, 0, 1) * k(1, 1); // extended

			out(i++) = v;
		}
	}

	// 0 < y < n
	while (i < cols * (rows - 1)) {
		// x = 0
		{
			auto v = casts::to<T>(0);

			v += d(i, 0, -1) * k(-1, -1); // extended
			v += d(i, 0, -1) * k(0, -1);
			v += d(i, 1, -1) * k(1, -1);

			v += d(i, 0, 0) * k(-1, 0); // extended
			v += d(i, 0, 0) * k(0, 0);
			v += d(i, 1, 0) * k(1, 0);

			v += d(i, 0, 1) * k(-1, 1); // extended
			v += d(i, 0, 1) * k(0, 1);
			v += d(i, 1, 1) * k(1, 1);

			out(i++) = v;
		}

		// 0 < x < n
		const auto limit = i + cols - 2;
		while (i < limit) {
			auto v = casts::to<T>(0);

			v += d(i, -1, -1) * k(-1, -1);
			v += d(i, 0, -1) * k(0, -1);
			v += d(i, 1, -1) * k(1, -1);

			v += d(i, -1, 0) * k(-1, 0);
			v += d(i, 0, 0) * k(0, 0);
			v += d(i, 1, 0) * k(1, 0);

			v += d(i, -1, 1) * k(-1, 1);
			v += d(i, 0, 1) * k(0, 1);
			v += d(i, 1, 1) * k(1, 1);

			out(i++) = v;
		}

		// x = n - 1
		{
			auto v = casts::to<T>(0);

			v += d(i, -1, -1) * k(-1, -1);
			v += d(i, 0, -1) * k(0, -1);
			v += d(i, 0, -1) * k(1, -1); // extended

			v += d(i, -1, 0) * k(-1, 0);
			v += d(i, 0, 0) * k(0, 0);
			v += d(i, 0, 0) * k(1, 0); // extended

			v += d(i, -1, 1) * k(-1, 1);
			v += d(i, 0, 1) * k(0, 1);
			v += d(i, 0, 1) * k(1, 1); // extended

			out(i++) = v;
		}
	}

	// y = n - 1
	{
		// x = 0
		{
			auto v = casts::to<T>(0);

			v += d(i, 0, -1) * k(-1, -1); // extended
			v += d(i, 0, -1) * k(0, -1);
			v += d(i, 1, -1) * k(1, -1);

			v += d(i, 0, 0) * k(-1, 0); // extended
			v += d(i, 0, 0) * k(0, 0);
			v += d(i, 1, 0) * k(1, 0);

			v += d(i, 0, 0) * k(-1, 1); // extended
			v += d(i, 0, 0) * k(0, 1);  // extended
			v += d(i, 1, 0) * k(1, 1);  // extended

			out(i++) = v;
		}

		// 1 < x < n - 2
		const auto limit = i + cols - 2;
		while (i < limit) {
			auto v = casts::to<T>(0);

			v += d(i, -1, -1) * k(-1, -1);
			v += d(i, 0, -1) * k(0, -1);
			v += d(i, 1, -1) * k(1, -1);

			v += d(i, -1, 0) * k(-1, 0);
			v += d(i, 0, 0) * k(0, 0);
			v += d(i, 1, 0) * k(1, 0);

			v += d(i, -1, 0) * k(-1, 1); // extended
			v += d(i, 0, 0) * k(0, 1);   // extended
			v += d(i, 1, 0) * k(1, 1);   // extended

			out(i++) = v;
		}

		// x = n - 1
		{
			auto v = casts::to<T>(0);

			v += d(i, -1, -1) * k(-1, -1);
			v += d(i, 0, -1) * k(0, -1);
			v += d(i, 0, -1) * k(1, -1); // extended

			v += d(i, -1, 0) * k(-1, 0);
			v += d(i, 0, 0) * k(0, 0);
			v += d(i, 0, 0) * k(1, 0); // extended

			v += d(i, -1, 0) * k(-1, 1); // extended
			v += d(i, 0, 0) * k(0, 1);   // extended
			v += d(i, 0, 0) * k(1, 1);   // extended

			out(i++) = v;
		}
	}
}

} // namespace iptsd::contacts::detection::convolution::impl
