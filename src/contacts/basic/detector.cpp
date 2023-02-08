// SPDX-License-Identifier: GPL-2.0-or-later

#include "detector.hpp"

#include "../interface.hpp"
#include "cluster.hpp"

#include <common/types.hpp>
#include <container/image.hpp>
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
	f32 neutral = this->neutral();

	// Mark positions where the value is 0 as visited to
	// avoid producing clusters spanning the whole map.
	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			if (this->heatmap[pos] > (neutral + (8 / 255.0f)))
				continue;

			this->visited[pos] = true;
		}
	}

	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			if (this->visited[pos])
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

f32 BlobDetector::neutral()
{
	index2_t size = this->heatmap.size();
	std::array<u32, UINT8_MAX + 1> count {};

	for (index_t x = 0; x < size.x; x++) {
		for (index_t y = 0; y < size.y; y++) {
			index2_t pos {x, y};

			u8 val = gsl::narrow_cast<u8>(this->heatmap[pos] * UINT8_MAX);
			count.at(val)++;
		}
	}

	auto max = std::max_element(count.begin(), count.end());
	u32 idx = std::distance(count.begin(), max);

	return gsl::narrow<f32>(idx + 1) / UINT8_MAX;
}

} // namespace iptsd::contacts::basic
