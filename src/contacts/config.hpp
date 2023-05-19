// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_CONFIG_HPP
#define IPTSD_CONTACTS_CONFIG_HPP

#include "detection/config.hpp"
#include "stability/config.hpp"
#include "validation/config.hpp"

#include <common/types.hpp>

namespace iptsd::contacts {

template <class T>
struct Config {
public:
	static_assert(std::is_floating_point_v<T>);

public:
	// The configuration options for the detection phase.
	detection::Config<T> detection {};

	// The configuration options for the validation phase.
	validation::Config<T> validation {};

	// The configuration options for the stabilization phase.
	stability::Config<T> stability {};
};

} // namespace iptsd::contacts

#endif // IPTSD_CONTACTS_CONFIG_HPP
