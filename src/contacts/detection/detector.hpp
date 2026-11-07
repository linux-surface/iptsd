// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_DETECTOR_HPP
#define IPTSD_CONTACTS_DETECTION_DETECTOR_HPP

#include "../contact.hpp"
#include "algorithms/cluster.hpp"
#include "algorithms/convolution.hpp"
#include "algorithms/ellipse.hpp"
#include "algorithms/gaussian.hpp"
#include "algorithms/kernels.hpp"
#include "algorithms/maximas.hpp"
#include "algorithms/neutral.hpp"
#include "algorithms/overlaps.hpp"
#include "algorithms/suppression.hpp"
#include "config.hpp"

#include <common/casts.hpp>
#include <common/types.hpp>

#include <gsl/gsl>

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

namespace iptsd::contacts::detection {

template <class T, class TFit = T>
class Detector {
public:
	static_assert(std::is_floating_point_v<T>);
	static_assert(std::is_floating_point_v<TFit>);

private:
	Config<T> m_config;

	// The diagonal of the heatmap.
	T m_input_diagonal = casts::to<T>(0);

	// The heatmap with the neutral value subtracted.
	Image<T> m_img_neutral {};

	// The blurred heatmap.
	Image<T> m_img_blurred {};

	// A copy of the blurred heatmap used for cluster spanning when suppression is enabled.
	Image<T> m_img_span {};

	// The kernel that is used for blurring.
	Matrix3<T> m_kernel_blur = kernels::gaussian<T, 3, 3>(gsl::narrow_cast<T>(0.75));

	// The list of local maximas.
	std::vector<Point> m_maximas {};

	// The list of spanned clusters.
	std::vector<Box> m_clusters {};

	// Temporary storage for cluster spanning.
	std::vector<Box> m_clusters_temp {};

	// Input parameters for gaussian fitting.
	std::vector<gaussian::Parameters<TFit>> m_fitting_params {};

	// Temporary storage for gaussian fitting.
	Image<TFit> m_fitting_temp {};

	// How many frames are left before the neutral value has to be recalculated.
	usize m_counter = 0;

	// The cached neutral value of the heatmap.
	T m_neutral = casts::to<T>(0);

	// Resting patterns per heatmap row, used when the neutral algorithm is ADAPTIVE.
	std::vector<std::vector<std::vector<T>>> m_templates {};

	// Scratch buffer for computing row distances.
	std::vector<T> m_scratch {};

public:
	explicit Detector(Config<T> config) : m_config {std::move(config)} {};

	/*!
	 * Search for contacts in a capacitive heatmap.
	 *
	 * This function uses a threshold based recursive descent to build
	 * a list of connected clusters based on the (pre-processed) heatmap,
	 * and then uses gaussian fitting to fit an ellipse onto these clusters.
	 *
	 * @param[in] heatmap The heatmap to process.
	 * @param[out] contacts The list of detected contacts.
	 */
	template <int Rows, int Cols>
	void detect(const ImageBase<T, Rows, Cols> &heatmap, std::vector<Contact<T>> &contacts)
	{
		const Vector2<Eigen::Index> one = Vector2<Eigen::Index>::Ones();

		const Eigen::Index cols = heatmap.cols();
		const Eigen::Index rows = heatmap.rows();

		const Vector2<Eigen::Index> dimensions {cols - 1, rows - 1};

		const Eigen::Index bcols = m_img_neutral.cols();
		const Eigen::Index brows = m_img_neutral.rows();

		// Resize the internal buffers if neccessary.
		if (brows != rows || bcols != cols) {
			m_img_neutral.conservativeResize(rows, cols);
			m_img_blurred.conservativeResize(rows, cols);
			m_img_span.conservativeResize(rows, cols);
			m_fitting_temp.conservativeResize(rows, cols);
			m_templates.assign(casts::to<usize>(rows), {});

			if (m_config.normalize)
				m_input_diagonal = std::hypot(dimensions.x(), dimensions.y());
		}

		contacts.clear();
		m_clusters.clear();
		m_fitting_params.clear();

		if (m_config.neutral_value_algorithm == neutral::Algorithm::ADAPTIVE) {
			this->subtract_adaptive(heatmap);
		} else {
			// Recalculate the neutral value if neccessary
			if (m_counter == 0) {
				m_neutral = neutral::calculate(heatmap,
				                               m_config.neutral_value_algorithm,
				                               m_config.neutral_value_offset);
			}

			// Update counter
			m_counter = (m_counter + 1) % m_config.neutral_value_backoff;

			// Subtract the neutral value from the whole heatmap
			m_img_neutral = (heatmap - m_neutral).max(casts::to<T>(0));
		}

		// Blur the heatmap slightly
		convolution::run(m_img_neutral, m_kernel_blur, m_img_blurred);

		const T athresh = m_config.activation_threshold;
		const T dthresh = m_config.deactivation_threshold;

		// Find local maximas on the original blurred heatmap (stable maxima positions)
		maximas::find(m_img_blurred, athresh, m_maximas);

		// Create a suppressed copy used only for cluster spanning
		const Image<T> *span_src = &m_img_blurred;

		const Eigen::Index suppression_radius =
			casts::to_eigen(m_config.peak_suppression_radius);
		if (suppression_radius >= 1) {
			const T suppression_factor = m_config.peak_suppression_factor;
			suppression::darken_around_maximas(m_img_blurred,
			                                   m_maximas,
			                                   suppression_radius,
			                                   suppression_factor,
			                                   m_img_span);

			span_src = &m_img_span;
		}

		// Iterate over the maximas and start building clusters (on suppressed image if
		// enabled)
		for (const Point &point : m_maximas) {
			Box cluster = cluster::span(*span_src, point, athresh, dthresh);

			if (cluster.isEmpty())
				continue;

			// Extend the sides of the cluster by one pixel
			cluster.min() = (cluster.min() - one).cwiseMax(0);
			cluster.max() = (cluster.max() + one).cwiseMin(dimensions);

			// min() and max() are inclusive so we need to add one
			const Vector2<Eigen::Index> size = cluster.sizes() + one;

			// For gaussian fitting, the clusters should have at least 3x3 pixels
			if (size.x() < 3 || size.y() < 3)
				continue;

			m_clusters.push_back(std::move(cluster));
		}

		// Merge overlapping clusters
		overlaps::merge(m_clusters, m_clusters_temp, 5);

		// Prepare clusters for gaussian fitting
		for (const Box &cluster : m_clusters) {
			const Vector2<TFit> mean = cluster.cast<TFit>().center();
			const Matrix2<TFit> prec = Matrix2<TFit>::Identity();

			// min() and max() are inclusive so we need to add one
			const Vector2<Eigen::Index> size = cluster.sizes() + one;

			gaussian::Parameters<TFit> params {
				true,
				1,
				mean,
				prec,
				cluster,
				Image<TFit> {size.y(), size.x()},
			};

			m_fitting_params.push_back(std::move(params));
		}

		// Run gaussian fitting
		gaussian::fit(m_fitting_params, m_img_blurred, m_fitting_temp, 3);

		// Create a contact from every gaussian fitting parameter
		for (const auto &p : m_fitting_params) {
			if (!p.valid)
				continue;

			const Matrix2<TFit> cov = p.prec.inverse();

			Eigen::SelfAdjointEigenSolver<Matrix2<TFit>> solver {};
			solver.computeDirect(cov);

			Vector2<TFit> mean = p.mean;
			Vector2<TFit> size = ellipse::size(solver.eigenvalues());
			TFit orientation = ellipse::angle<TFit>(solver.eigenvectors());

			// Normalize dimensions.
			if (m_config.normalize) {
				mean = mean.cwiseQuotient(dimensions.cast<TFit>());
				size = (size.array() / m_input_diagonal).matrix();
				orientation /= gsl::narrow_cast<TFit>(M_PI);
			}

			contacts.push_back(Contact<T> {
				mean.template cast<T>(),
				size.template cast<T>(),
				gsl::narrow_cast<T>(orientation),
				m_config.normalize,
			});
		}
	}

private:
	/*!
	 * Subtracts a learned resting pattern from every row of the heatmap.
	 *
	 * Each row keeps up to MaxTemplates resting patterns. For every frame the pattern
	 * with the smallest median absolute difference is chosen; a fingertip only covers
	 * a few columns, so it barely affects the median and the right pattern is still
	 * found. Rows that match no pattern start a new one. Patterns adapt slowly, but
	 * only in cells that are close to them, so contacts are never absorbed.
	 *
	 * @param[in] heatmap The current heatmap.
	 */
	template <int Rows, int Cols>
	void subtract_adaptive(const ImageBase<T, Rows, Cols> &heatmap)
	{
		constexpr usize MaxTemplates = 4;
		const T match = casts::to<T>(6.0 / 255.0);
		const T quiet = m_config.activation_threshold / casts::to<T>(2);
		const T rate = casts::to<T>(0.02);

		const Eigen::Index cols = heatmap.cols();
		m_scratch.resize(casts::to<usize>(cols));

		for (Eigen::Index y = 0; y < heatmap.rows(); y++) {
			auto &templates = m_templates.at(casts::to<usize>(y));

			// Find the closest resting pattern by median absolute difference.
			usize best = 0;
			T best_dist = std::numeric_limits<T>::max();

			for (usize k = 0; k < templates.size(); k++) {
				for (Eigen::Index x = 0; x < cols; x++)
					m_scratch.at(casts::to<usize>(x)) =
						std::abs(heatmap(y, x) -
						         templates.at(k).at(casts::to<usize>(x)));

				const auto mid = m_scratch.begin() + (cols / 2);
				std::nth_element(m_scratch.begin(), mid, m_scratch.end());

				if (*mid < best_dist) {
					best_dist = *mid;
					best = k;
				}
			}

			// Start a new pattern if none matches.
			if (best_dist > match && templates.size() < MaxTemplates) {
				templates.emplace_back(casts::to<usize>(cols));
				for (Eigen::Index x = 0; x < cols; x++)
					templates.back().at(casts::to<usize>(x)) = heatmap(y, x);
				best = templates.size() - 1;
			}

			auto &pattern = templates.at(best);

			for (Eigen::Index x = 0; x < cols; x++) {
				T &base = pattern.at(casts::to<usize>(x));
				const T diff = heatmap(y, x) - base;

				m_img_neutral(y, x) = std::max(diff - m_config.neutral_value_offset,
				                               casts::to<T>(0));

				// Adapt slowly, but never towards a contact.
				if (diff < quiet)
					base += diff * rate;
			}
		}
	}
};

} // namespace iptsd::contacts::detection

#endif // IPTSD_CONTACTS_DETECTION_DETECTOR_HPP
