// SPDX-License-Identifier: GPL-2.0-or-later

#include "detector.hpp"

#include "../interface.hpp"
#include "../neutral.hpp"
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
	std::fill(this->visited.begin(), this->visited.end(), false);
	this->blobs.clear();

	index2_t size = this->heatmap.size();

	f32 nval = neutral(this->config, this->heatmap);
	f32 activate = nval + (this->config.activation_threshold / 255);
	f32 deactivate = nval + (this->config.deactivation_threshold / 255);

	// Mark positions where the blob detection should not be active as visited.
	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			if (this->heatmap[pos] > deactivate)
				continue;

			this->visited[pos] = true;
		}
	}

	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			if (this->visited[pos])
				continue;

			if (this->heatmap[pos] < activate)
				continue;

			Cluster cluster {this->heatmap, this->visited, pos};

			math::Mat2s<f64> cov = cluster.cov();
			math::Eigen2<f64> eigen = cov.eigen();

			if (eigen.w[0] <= 0 || eigen.w[1] <= 0)
				continue;

			if (std::isnan(eigen.v[0].x) || std::isnan(eigen.v[0].y) ||
			    std::isnan(eigen.v[1].x) || std::isnan(eigen.v[1].x))
				continue;

			this->blobs.push_back(Blob {cluster.mean() + 0.5, cov});
		}
	}

	return this->blobs;
}

} // namespace iptsd::contacts::basic
