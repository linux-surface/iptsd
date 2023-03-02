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

class BlobDetector : public IBlobDetector {
private:
	BlobDetectorConfig config;

	container::Image<f32> heatmap;

	std::vector<index2_t> maximas {64};
	std::vector<Cluster> clusters {};
	std::vector<Cluster> temp {};

	container::Image<f64> gfit_temp;
	std::vector<advanced::alg::gfit::Parameters<f64>> gfit_params {};

	std::vector<Blob> blobs {64};

public:
	BlobDetector(const index2_t size, const BlobDetectorConfig config)
		: config {config}, heatmap {size}, gfit_temp {size} {};

	container::Image<f32> &data() override;
	const std::vector<Blob> &search() override;
};

inline container::Image<f32> &BlobDetector::data()
{
	return this->heatmap;
}

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_DETECTOR_HPP */
