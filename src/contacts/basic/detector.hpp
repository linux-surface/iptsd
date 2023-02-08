/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_DETECTOR_HPP
#define IPTSD_CONTACTS_BASIC_DETECTOR_HPP

#include "../interface.hpp"

#include <common/types.hpp>
#include <container/image.hpp>

namespace iptsd::contacts::basic {

class BlobDetector : public IBlobDetector {
private:
	BlobDetectorConfig config;

	container::Image<f32> heatmap;
	container::Image<bool> visited;

	std::vector<Blob> blobs {};

public:
	BlobDetector(index2_t size, BlobDetectorConfig config)
		: config {config}, heatmap {size}, visited {size} {};

	container::Image<f32> &data() override;
	const std::vector<Blob> &search() override;
};

inline container::Image<f32> &BlobDetector::data()
{
	return this->heatmap;
}

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_DETECTOR_HPP */
