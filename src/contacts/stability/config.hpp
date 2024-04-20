// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_STABILITY_CONFIG_HPP
#define IPTSD_CONTACTS_STABILITY_CONFIG_HPP

#include <common/types.hpp>

#include <optional>
#include <type_traits>

namespace iptsd::contacts::stability {

template <class T>
struct Config {
public:
	static_assert(std::is_floating_point_v<T>);

public:
	/*
	 * The limits that the size difference of a contact between two frames may not exceed.
	 */
	std::optional<Vector2<T>> size_threshold = std::nullopt;

	/*
	 * The limits that the position difference of a contact between two frames may not exceed.
	 */
	std::optional<Vector2<T>> position_threshold = std::nullopt;

	/*
	 * The limits that the orientation difference of a contact between two frames may not
	 * exceed.
	 */
	std::optional<Vector2<T>> orientation_threshold = std::nullopt;
};

} // namespace iptsd::contacts::stability

#endif // IPTSD_CONTACTS_STABILITY_CONFIG_HPP
