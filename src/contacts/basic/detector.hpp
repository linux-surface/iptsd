/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_DETECTOR_HPP
#define IPTSD_CONTACTS_BASIC_DETECTOR_HPP

#include "../advanced/algorithm/convolution.hpp"
#include "../advanced/algorithm/gaussian_fitting.hpp"
#include "../interface.hpp"
#include "cluster.hpp"

#include <common/types.hpp>
#include <container/image.hpp>
#include <container/kernel.hpp>

#include <vector>

namespace iptsd::contacts::basic {

using namespace iptsd::contacts::advanced::alg;

class BlobDetector : public IBlobDetector {
private:
	BlobDetectorConfig config;

	container::Image<f32> heatmap;
	container::Image<f32> blurred;
	container::Image<f32> neutralized;
	container::Image<f64> fitting;

	container::Kernel<f32, 3, 3> gaussian;

	std::vector<index2_t> maximas {64};
	std::vector<Cluster> clusters {};
	std::vector<Cluster> temp {};
	std::vector<gfit::Parameters<f64>> params {};

	std::vector<Blob> blobs {64};

public:
	BlobDetector(const index2_t size, const BlobDetectorConfig config)
		: config {config}, heatmap {size}, blurred {size}, neutralized {size},
		  fitting {size}, gaussian {conv::kernels::gaussian<f32, 3, 3>(0.75f)} {};

	container::Image<f32> &data() override;
	const std::vector<Blob> &search() override;
};

inline container::Image<f32> &BlobDetector::data()
{
	return this->heatmap;
}

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_DETECTOR_HPP */
