/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_DETECTOR_HPP
#define IPTSD_CONTACTS_BASIC_DETECTOR_HPP

#include "../advanced/algorithm/gaussian_fitting.hpp"
#include "../interface.hpp"
#include "cluster.hpp"

#include <common/types.hpp>
#include <container/image.hpp>

#include <vector>

namespace iptsd::contacts::basic {

using namespace iptsd::contacts::advanced::alg;

class BlobDetector : public IBlobDetector {
private:
	BlobDetectorConfig config;

	container::Image<f32> heatmap;
	container::Image<f32> neutralized;
	container::Image<f64> fitting;

	std::vector<index2_t> maximas {64};
	std::vector<Cluster> clusters {};
	std::vector<Cluster> temp {};
	std::vector<gfit::Parameters<f64>> params {};

	std::vector<Blob> blobs {64};

public:
	BlobDetector(const index2_t size, const BlobDetectorConfig config)
		: config {config}, heatmap {size}, neutralized {size}, fitting {size} {};

	container::Image<f32> &data() override;
	const std::vector<Blob> &search() override;
};

inline container::Image<f32> &BlobDetector::data()
{
	return this->heatmap;
}

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_DETECTOR_HPP */
