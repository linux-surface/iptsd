// SPDX-License-Identifier: Apache-2.0

#ifndef IPTSD_PREDICTION_KALMAN_FILTER_HPP
#define IPTSD_PREDICTION_KALMAN_FILTER_HPP

#include <common/types.hpp>

namespace iptsd::prediction {

/*
 * Kalman filter implementation following http://filterpy.readthedocs.io/en/latest/
 *
 * To improve performance, this filter is specialized to use a 4 dimensional state, with single
 * dimension measurements.
 */
class KalmanFilter {
public:
	/*
	 * State estimate
	 */
	Matrix<f64> x;

	/*
	 * State estimate covariance
	 */
	Matrix<f64> P;

	/*
	 * Process noise
	 */
	Matrix<f64> Q;

	/*
	 * Measurement noise (mZDim, mZDim)
	 */
	Matrix<f64> R;

	/*
	 * State transition matrix
	 */
	Matrix<f64> F;

	/*
	 * Measurement matrix
	 */
	Matrix<f64> H;

	/*
	 * Kalman gain
	 */
	Matrix<f64> K;

public:
	KalmanFilter(const Eigen::Index x_dim, const Eigen::Index z_dim)
		: x {x_dim, 1}
		, P {Matrix<f64>::Identity(x_dim, x_dim)}
		, Q {Matrix<f64>::Identity(x_dim, x_dim)}
		, R {Matrix<f64>::Identity(z_dim, z_dim)}
		, F {x_dim, x_dim}
		, H {z_dim, x_dim}
		, K {x_dim, z_dim} {};

	/*!
	 * Resets the internal state of this Kalman filter.
	 */
	void reset()
	{
		// NOTE: It is not necessary to reset Q, R, F, and H matrices.
		x.fill(0);
		P.setIdentity();
		K.fill(0);
	}

	/*!
	 * Performs the prediction phase of the filter, using the state estimate to produce a new
	 * estimate for the current timestep.
	 */
	void predict()
	{
		x = F * x;
		P = ((F * P) * F.transpose()) + Q;
	}

	/*!
	 * Updates the state estimate to incorporate the new observation z.
	 */
	void update(const Matrix<f64> &z)
	{
		const auto y = z - (H * x);
		const auto tS = ((H * P) * H.transpose()) + R;

		K = (P * H.transpose()) * tS.inverse();
		x = x + (K * y);
		P = P - ((K * H) * P);
	}
};

} // namespace iptsd::prediction

#endif // IPTSD_PREDICTION_KALMAN_FILTER_HPP
