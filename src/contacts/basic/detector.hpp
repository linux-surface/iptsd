/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_DETECTOR_HPP
#define IPTSD_CONTACTS_BASIC_DETECTOR_HPP

#include "../interface.hpp"
#include "heatmap.hpp"

#include <common/types.hpp>
#include <container/image.hpp>

namespace iptsd::contacts::basic {

class BlobDetector : public IBlobDetector {
private:
	Heatmap heatmap;
	std::vector<Blob> blobs {};

public:
	BlobDetector(index2_t size) : heatmap {size} {};

	container::Image<f32> &data() override;
	const std::vector<Blob> &search() override;
};

inline container::Image<f32> &BlobDetector::data()
{
	return this->heatmap.data;
}

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_DETECTOR_HPP */
