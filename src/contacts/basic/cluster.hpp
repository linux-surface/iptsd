/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_CLUSTER_HPP
#define IPTSD_CONTACTS_BASIC_CLUSTER_HPP

#include <common/types.hpp>
#include <container/image.hpp>
#include <math/mat2.hpp>
#include <math/vec2.hpp>

#include <limits>
#include <type_traits>
#include <vector>

namespace iptsd::contacts::basic {

class Cluster {
public:
	index_t max_x = 0;
	index_t max_y = 0;
	index_t min_x = std::numeric_limits<index_t>::max();
	index_t min_y = std::numeric_limits<index_t>::max();

private:
	container::Image<bool> visited;

public:
	Cluster(index2_t size);
	Cluster(const Cluster &other) = default;
	Cluster(Cluster &&other) noexcept = default;

	void add(index2_t position);
	void merge(const Cluster &other);

	[[nodiscard]] bool contains(index2_t position) const;

	[[nodiscard]] index2_t min() const;
	[[nodiscard]] index2_t max() const;
	[[nodiscard]] index2_t size() const;
};

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_CLUSTER_HPP */
