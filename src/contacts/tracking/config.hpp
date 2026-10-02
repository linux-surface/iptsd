// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_TRACKING_CONFIG_HPP
#define IPTSD_CONTACTS_TRACKING_CONFIG_HPP

#include <common/casts.hpp>
#include <common/types.hpp>

#include <type_traits>

namespace iptsd::contacts::tracking {

template <class T>
struct Config {
public:
	static_assert(std::is_floating_point_v<T>);

public:
	/*
	 * For how many frames a contact that was not detected anymore is kept alive.
	 *
	 * If the contact shows up again during that time, it keeps its index and the
	 * touch is not interrupted. Setting this to zero disables the feature.
	 */
	usize retain_frames = 0;

	/*
	 * How far a retained contact may have moved when it shows up again.
	 *
	 * New contacts that are further away from a retained contact are not matched to it.
	 */
	T retain_distance = casts::to<T>(0);

	/*
	 * The position of a contact is multiplied with this before measuring retain_distance.
	 *
	 * For normalized contacts this is the size of the screen, so that the distance is
	 * measured in the same unit along both axes.
	 */
	Vector2<T> retain_scale = Vector2<T>::Ones();
};

} // namespace iptsd::contacts::tracking

#endif // IPTSD_CONTACTS_TRACKING_CONFIG_HPP
