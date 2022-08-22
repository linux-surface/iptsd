// SPDX-License-Identifier: GPL-2.0-or-later

#include "detector.hpp"

#include "../interface.hpp"
#include "cluster.hpp"

#include <common/types.hpp>
#include <container/image.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <gsl/gsl>
#include <vector>

namespace iptsd::contacts::basic {

const std::vector<Blob> &BlobDetector::search()
{
	this->heatmap.reset();
	this->blobs.clear();

	index2_t size = this->heatmap.data.size();

	// Mark positions where the value is 0 as visited to
	// avoid producing clusters spanning the whole map.
	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			if (this->heatmap.value(pos) > 0)
				continue;

			this->heatmap.set_visited(pos, true);
		}
	}

	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			if (this->heatmap.get_visited(pos))
				continue;

			Cluster cluster {this->heatmap, pos};

			math::Mat2s<f32> cov = cluster.cov();
			math::Eigen2<f32> eigen = cov.eigen();

			if (eigen.w[0] <= 0 || eigen.w[1] <= 0)
				continue;

			this->blobs.push_back(Blob {cluster.mean() + 0.5f, cov});
		}
	}

	return this->blobs;
}

} // namespace iptsd::contacts::basic
