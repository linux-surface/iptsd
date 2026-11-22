// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_STABILITY_CONFIG_HPP
#define IPTSD_CONTACTS_STABILITY_CONFIG_HPP

#include <common/casts.hpp>
#include <common/types.hpp>

#include <gsl/gsl>

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

	/*
	 * Optional One-Euro low-pass filter on contact position (replaces the hard
	 * PositionThresholdMin deadzone freeze with adaptive speed-based smoothing).
	 */
	bool one_euro_enabled = false;
	T one_euro_min_cutoff = gsl::narrow_cast<T>(1.5);
	T one_euro_beta = gsl::narrow_cast<T>(0.4);
	T one_euro_d_cutoff = casts::to<T>(1);

	/*
	 * Physical dimensions (width, height in cm) used to compute speed in cm/s
	 * for the One-Euro filter.
	 */
	Vector2<T> physical_size = Vector2<T>::Ones();
};

} // namespace iptsd::contacts::stability

#endif // IPTSD_CONTACTS_STABILITY_CONFIG_HPP
