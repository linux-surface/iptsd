// SPDX-License-Identifier: GPL-2.0-or-later

#include "detector.hpp"

#include "../interface.hpp"
#include "../neutral.hpp"
#include "algorithms.hpp"
#include "cluster.hpp"

#include <common/types.hpp>
#include <container/image.hpp>
#include <container/ops.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <algorithm>
#include <cmath>
#include <gsl/gsl>
#include <vector>

namespace iptsd::contacts::basic {

const std::vector<Blob> &BlobDetector::search()
{
	this->maximas.clear();
	this->blobs.clear();

	const f32 nval = neutral(this->config, this->heatmap);
	const f32 athresh = nval + (this->config.activation_threshold / 255);
	const f32 dthresh = nval + (this->config.deactivation_threshold / 255);

	// Search for local maxima
	algorithms::find_local_maximas(this->heatmap, athresh, this->maximas);

	// Iterate over the maximas and start building clusters
	for (const index2_t point : this->maximas) {
		const Cluster cluster = algorithms::span_cluster(this->heatmap, athresh, dthresh, point);

		const math::Mat2s<f64> cov = cluster.cov();
		const math::Eigen2<f64> eigen = cov.eigen();

		if (eigen.w[0] <= 0 || eigen.w[1] <= 0)
			continue;

		if (std::isnan(eigen.v[0].x) || std::isnan(eigen.v[0].y) ||
		    std::isnan(eigen.v[1].x) || std::isnan(eigen.v[1].x))
			continue;

		this->blobs.push_back(Blob {cluster.mean() + 0.5, cov});
	}

	return this->blobs;
}

} // namespace iptsd::contacts::basic
