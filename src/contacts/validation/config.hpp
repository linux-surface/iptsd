// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_VALIDATION_CONFIG_HPP
#define IPTSD_CONTACTS_VALIDATION_CONFIG_HPP

#include <common/types.hpp>

#include <optional>

namespace iptsd::contacts::validation {

template <class T>
struct Config {
public:
	static_assert(std::is_floating_point_v<T>);

public:
	/*
	 * Whether the validity should be tracked over multiple frames.
	 *
	 * If this option is set to true, a contact that was marked as invalid
	 * once will stay invalid until it is lifted.
	 */
	bool track_validity = true;

	/*
	 * The limits that the aspect ratio of a valid contact must not exceed.
	 */
	std::optional<Vector2<T>> aspect_limits = std::nullopt;

	/*
	 * The limits that the size of a valid contact must not exceed.
	 */
	std::optional<Vector2<T>> size_limits = std::nullopt;
};

} // namespace iptsd::contacts::validation

#endif // IPTSD_CONTACTS_VALIDATION_CONFIG_HPP
