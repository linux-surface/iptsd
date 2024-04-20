// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_GAUSSIAN_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_GAUSSIAN_HPP

#include <common/casts.hpp>
#include <common/types.hpp>

#include <gsl/gsl>
#include <gsl/util>

#include <type_traits>

namespace iptsd::contacts::detection::gaussian {

template <class T>
struct Parameters {
	// Flag to invalidate parameters.
	bool valid = false;

	// alpha
	T scale = 0;

	// mu
	Vector2<T> mean;

	// precision matrix, aka. inverse covariance matrix, aka. sigma^-1
	Matrix2<T> prec;

	// local bounds for sampling
	Box bounds;

	// local weights for sampling
	Matrix<T> weights;
};

namespace impl {

template <class T>
constexpr T EPS = std::is_same_v<T, f32> ? gsl::narrow_cast<T>(1E-20) : gsl::narrow_cast<T>(1E-40);

/*!
 * 2D Gaussian probability density function without normalization.
 *
 * @param[in] x Position at which to evaluate the function.
 * @param[in] mean Mean of the Gaussian.
 * @param[in] prec Precision matrix, i.e. the invariance of the covariance matrix.
 */
template <class T>
T gaussian_like(const Vector2<T> &x, const Vector2<T> &mean, const Matrix2<T> &prec)
{
	const Vector2<T> vec = x - mean;
	const T vtmv = vec.transpose() * prec * vec;

	return std::exp(-vtmv) / casts::to<T>(2);
}

template <class T, class DerivedData>
void assemble_system(Matrix6<T> &m,
                     Vector6<T> &rhs,
                     const Box &b,
                     const DenseBase<DerivedData> &data,
                     const Matrix<T> &w)
{
	const Eigen::Index cols = data.cols();
	const Eigen::Index rows = data.rows();

	const auto scale = Vector2<T> {
		casts::to<T>(2) / casts::to<T>(cols),
		casts::to<T>(2) / casts::to<T>(rows),
	};

	m.setZero();
	rhs.setZero();

	const Point &bmin = b.min();
	const Point &bmax = b.max();

	for (Eigen::Index iy = bmin.y(); iy <= bmax.y(); iy++) {
		const T y = casts::to<T>(iy) * scale.y() - 1;

		for (Eigen::Index ix = bmin.x(); ix <= bmax.x(); ix++) {
			const T x = casts::to<T>(ix) * scale.x() - 1;

			const T d =
				w(iy - bmin.y(), ix - bmin.x()) * gsl::narrow_cast<T>(data(iy, ix));
			const T v = std::log(d + EPS<T>) * d * d;

			rhs(0) += v * x * x;
			rhs(1) += v * x * y;
			rhs(2) += v * y * y;
			rhs(3) += v * x;
			rhs(4) += v * y;
			rhs(5) += v;

			m(0, 0) += d * d * x * x * x * x;
			m(0, 1) += d * d * x * x * x * y;
			m(0, 2) += d * d * x * x * y * y;
			m(0, 3) += d * d * x * x * x;
			m(0, 4) += d * d * x * x * y;
			m(0, 5) += d * d * x * x;

			m(1, 0) += d * d * x * x * x * y;
			m(1, 1) += d * d * x * x * y * y;
			m(1, 2) += d * d * x * y * y * y;
			m(1, 3) += d * d * x * x * y;
			m(1, 4) += d * d * x * y * y;
			m(1, 5) += d * d * x * y;

			m(2, 0) += d * d * x * x * y * y;
			m(2, 1) += d * d * x * y * y * y;
			m(2, 2) += d * d * y * y * y * y;
			m(2, 3) += d * d * x * y * y;
			m(2, 4) += d * d * y * y * y;
			m(2, 5) += d * d * y * y;

			m(3, 0) += d * d * x * x * x;
			m(3, 1) += d * d * x * x * y;
			m(3, 2) += d * d * x * y * y;
			m(3, 3) += d * d * x * x;
			m(3, 4) += d * d * x * y;
			m(3, 5) += d * d * x;

			m(4, 0) += d * d * x * x * y;
			m(4, 1) += d * d * x * y * y;
			m(4, 2) += d * d * y * y * y;
			m(4, 3) += d * d * x * y;
			m(4, 4) += d * d * y * y;
			m(4, 5) += d * d * y;

			m(5, 0) += d * d * x * x;
			m(5, 1) += d * d * x * y;
			m(5, 2) += d * d * y * y;
			m(5, 3) += d * d * x;
			m(5, 4) += d * d * y;
			m(5, 5) += d * d;
		}
	}

	m.row(1) *= 2;
}

template <class T>
bool extract_params(const Vector6<T> &chi, T &scale, Vector2<T> &mean, Matrix2<T> &prec)
{
	prec.noalias() = (Matrix2<T> {} << chi[0], chi[1], chi[1], chi[2]).finished() * -2;

	// mu = sigma * b = prec^-1 * B
	const T d = prec.determinant();
	if (std::abs(d) <= EPS<T>)
		return false;

	mean.x() = (prec(1, 1) * chi[3] - prec(1, 0) * chi[4]) / d;
	mean.y() = (prec(0, 0) * chi[4] - prec(0, 1) * chi[3]) / d;

	const T vtmv = mean.transpose() * prec * mean;
	scale = std::exp(chi[5] + vtmv / casts::to<T>(2));

	return true;
}

template <class Derived>
void update_weight_maps(std::vector<Parameters<typename DenseBase<Derived>::Scalar>> &params,
                        DenseBase<Derived> &total)
{
	using T = typename DenseBase<Derived>::Scalar;

	const Eigen::Index cols = total.cols();
	const Eigen::Index rows = total.rows();

	const auto scale = Vector2<T> {
		casts::to<T>(2) / casts::to<T>(cols),
		casts::to<T>(2) / casts::to<T>(rows),
	};

	total.setZero();

	// compute individual Gaussians in sample windows
	for (auto &p : params) {
		if (!p.valid)
			continue;

		const Point bmin = p.bounds.min();
		const Point bmax = p.bounds.max();

		for (Eigen::Index iy = bmin.y(); iy <= bmax.y(); iy++) {
			const T y = casts::to<T>(iy) * scale.y() - 1;

			for (Eigen::Index ix = bmin.x(); ix <= bmax.x(); ix++) {
				const T x = casts::to<T>(ix) * scale.x() - 1;

				const T v = p.scale * gaussian_like({x, y}, p.mean, p.prec);
				p.weights(iy - bmin.y(), ix - bmin.x()) = v;
			}
		}
	}

	// sum up total
	for (auto &p : params) {
		if (!p.valid)
			continue;

		const Point bmin = p.bounds.min();
		const Point bmax = p.bounds.max();

		for (Eigen::Index y = bmin.y(); y <= bmax.y(); y++) {
			for (Eigen::Index x = bmin.x(); x <= bmax.x(); x++)
				total(y, x) += p.weights(y - bmin.y(), x - bmin.x());
		}
	}

	// normalize weights
	for (auto &p : params) {
		if (!p.valid)
			continue;

		const Point bmin = p.bounds.min();
		const Point bmax = p.bounds.max();

		for (Eigen::Index y = bmin.y(); y <= bmax.y(); y++) {
			for (Eigen::Index x = bmin.x(); x <= bmax.x(); x++) {
				const T t = total(y, x);

				if (t > casts::to<T>(0))
					p.weights(y - bmin.y(), x - bmin.x()) /= t;
			}
		}
	}
}

/**
 * ge_solve() - Solve a system of linear equations via Gaussian elimination.
 * @a: The system matrix A.
 * @b: The right-hand-side vector b.
 * @x: The vector to solve for.
 *
 * Solves the system of linear equations Ax = b using Gaussian elimination
 * with partial pivoting.
 */
template <class T>
bool ge_solve(Matrix6<T> a, Vector6<T> b, Vector6<T> &x)
{
	// TODO: optimize/unroll?

	// step 1: Gaussian elimination
	for (Eigen::Index c = 0; c < 6 - 1; ++c) {
		// partial pivoting for current column:
		// swap row r >= c with largest absolute value at [r, c] (i.e. in column) with row c
		{
			// step 1: find element with largest absolute value in column
			Eigen::Index r = 0;
			T v = casts::to<T>(0);

			for (Eigen::Index i = c; i < 6; ++i) {
				const T vi = std::abs(a(c, i));

				if (v < vi) {
					v = vi;
					r = i;
				}
			}

			// step 1.5: abort if we cannot find a sufficiently large pivot
			if (v <= EPS<T>)
				return false;

			// step 2: permutate, swap row r and c
			if (r != c) {
				for (Eigen::Index i = c; i < 6; ++i)
					std::swap(a(i, r), a(i, c)); // swap A[r, :] and A[c, :]

				std::swap(b[r], b[c]); // swap b[r] and b[c]
			}
		}

		// Gaussian elimination step
		for (Eigen::Index r = c + 1; r < 6; ++r) {
			const T v = a(c, r) / a(c, c);

			// b[r] = b[r] - (A[r, c] / A[c, c]) * b[c]
			b[r] -= v * b[c];

			// A[r, :] = A[r, :] - (A[r, c] / A[c, c]) * A[c, :]
			for (Eigen::Index k = c + 1; k < 6; ++k)
				a(k, r) -= v * a(k, c);
		}
	}

	// last check for r=5, c=5 because we've skipped that above
	if (std::abs(a(5, 5)) <= EPS<T>)
		return false;

	// step 2: backwards substitution
	x[5] = b[5];
	x[5] /= a(5, 5);

	x[4] = b[4] - a(5, 4) * x[5];
	x[4] /= a(4, 4);

	x[3] = b[3] - a(5, 3) * x[5] - a(4, 3) * x[4];
	x[3] /= a(3, 3);

	x[2] = b[2] - a(5, 2) * x[5] - a(4, 2) * x[4] - a(3, 2) * x[3];
	x[2] /= a(2, 2);

	x[1] = b[1] - a(5, 1) * x[5] - a(4, 1) * x[4] - a(3, 1) * x[3] - a(2, 1) * x[2];
	x[1] /= a(1, 1);

	x[0] = b[0] - a(5, 0) * x[5] - a(4, 0) * x[4] - a(3, 0) * x[3] - a(2, 0) * x[2] -
	       a(1, 0) * x[1];
	x[0] /= a(0, 0);

	return true;
}

} // namespace impl

template <class Derived, class DerivedData>
void fit(std::vector<Parameters<typename DenseBase<Derived>::Scalar>> &params,
         const DenseBase<DerivedData> &data,
         DenseBase<Derived> &tmp,
         const usize iterations)
{
	using T = typename DenseBase<Derived>::Scalar;

	const Eigen::Index cols = data.cols();
	const Eigen::Index rows = data.rows();

	const auto scale = Vector2<T> {
		casts::to<T>(2) / casts::to<T>(cols),
		casts::to<T>(2) / casts::to<T>(rows),
	};

	// down-scaling
	for (auto &p : params) {
		if (!p.valid)
			continue;

		// scale and center mean
		p.mean.array() = (p.mean.array() * scale.array()) - 1;

		// scale precision matrix (compute (S * Sigma * S^T)^-1 = S^-T * Prec * S^-1)
		p.prec.row(0) /= scale.x();
		p.prec.row(1) /= scale.y();
		p.prec.col(0) /= scale.x();
		p.prec.col(1) /= scale.y();
	}

	// perform iterations
	for (usize i = 0; i < iterations; ++i) {
		// update weights
		impl::update_weight_maps(params, tmp);

		// fit individual parameters
		for (auto &p : params) {
			Matrix6<T> sys {};
			Vector6<T> rhs {};
			Vector6<T> chi {};

			if (!p.valid)
				continue;

			// assemble system of linear equations
			impl::assemble_system(sys, rhs, p.bounds, data, p.weights);

			// solve systems
			p.valid = impl::ge_solve(sys, rhs, chi);
			if (!p.valid)
				continue;

			// get parameters
			p.valid = impl::extract_params(chi, p.scale, p.mean, p.prec);
		}
	}

	// undo down-scaling
	for (auto &p : params) {
		if (!p.valid)
			continue;

		// un-scale and re-adjust mean
		p.mean.array() = (p.mean.array() + 1) / scale.array();

		// un-scale precision matrix
		p.prec.row(0) *= scale.x();
		p.prec.row(1) *= scale.y();
		p.prec.col(0) *= scale.x();
		p.prec.col(1) *= scale.y();
	}
}

} // namespace iptsd::contacts::detection::gaussian

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_GAUSSIAN_HPP
