/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_ALGORITHMS_HPP
#define IPTSD_CONTACTS_BASIC_ALGORITHMS_HPP

#include "cluster.hpp"

#include <common/types.hpp>
#include <container/image.hpp>

#include <vector>

namespace iptsd::contacts::basic::algorithms {

void find_local_maximas(const container::Image<f32> &data, const f32 threshold,
			std::vector<index2_t> &out);

Cluster span_cluster(const container::Image<f32> &data, const f32 athresh, const f32 dthresh,
		     const index2_t center);

} /* namespace iptsd::contacts::basic::algorithms */

#endif /* IPTSD_CONTACTS_BASIC_ALGORITHMS_HPP */
