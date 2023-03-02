// SPDX-License-Identifier: GPL-2.0-or-later

#include "detector.hpp"

#include "../interface.hpp"
#include "../neutral.hpp"
#include "algorithms.hpp"
#include "cluster.hpp"

#include <common/types.hpp>
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
	this->clusters.clear();
	this->blobs.clear();

	const f32 nval = neutral(this->config, this->heatmap);
	const f32 athresh = nval + (this->config.activation_threshold / 255);
	const f32 dthresh = nval + (this->config.deactivation_threshold / 255);

	// Search for local maxima
	algorithms::find_local_maximas(this->heatmap, athresh, this->maximas);

	// Iterate over the maximas and start building clusters
	for (const index2_t point : this->maximas) {
		Cluster cluster = algorithms::span_cluster(this->heatmap, athresh, dthresh, point);

		this->clusters.push_back(std::move(cluster));
	}

	// Merge overlapping clusters
	algorithms::merge_overlaps(this->clusters, this->temp, 5);

	return this->blobs;
}

} // namespace iptsd::contacts::basic
