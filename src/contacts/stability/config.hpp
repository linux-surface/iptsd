// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_STABILITY_CONFIG_HPP
#define IPTSD_CONTACTS_STABILITY_CONFIG_HPP

#include <common/constants.hpp>
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
	 * How many frames are stored and considered for contact stability, including
	 * the current frame. This must be at least 2 for stability checking to work.
	 */
	usize temporal_window = 2;

	/*
	 * Whether the same index has to be present in all frames of the temporal window.
	 */
	bool check_temporal_stability = false;

	/*
	 * How much a contact can increase in size before it is considered unstable.
	 */
	std::optional<T> size_difference_threshold = std::nullopt;

	/*
	 * The limits that the movement of a stable contact between two frames may not exceed.
	 */
	std::optional<Vector2<T>> movement_limits = std::nullopt;

	/*
	 * How close a stable contact can be to an invalid one before getting marked as unstable.
	 */
	std::optional<T> distance_threshold = std::nullopt;
};

} // namespace iptsd::contacts::stability

#endif // IPTSD_CONTACTS_STABILITY_CONFIG_HPP
