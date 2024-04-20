// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_CONTACT_HPP
#define IPTSD_CONTACTS_CONTACT_HPP

#include <common/casts.hpp>
#include <common/types.hpp>

#include <optional>
#include <vector>

namespace iptsd::contacts {

template <class T>
class Contact {
public:
	static_assert(std::is_floating_point_v<T>);

public:
	/*
	 * The center position of the contact.
	 *
	 * Range: [0, 1] if normalized, [0, <input dimensions>] if not.
	 */
	Vector2<T> mean = Vector2<T>::Zero();

	/*
	 * The size of the contact (diameter of major and minor axis).
	 *
	 * Range: [0, 1] if normalized, [0, <hypot of input dimensions>] if not.
	 */
	Vector2<T> size = Vector2<T>::Zero();

	/*
	 * The orientation of the contact.
	 *
	 * Range: [0, 1) if normalized, [0, pi) if not.
	 */
	T orientation = casts::to<T>(0);

	/*
	 * Whether the stored values are normalized.
	 */
	bool normalized = false;

	/*
	 * A temporally stable index to track contacts over multiple frames.
	 */
	std::optional<usize> index = std::nullopt;

	/*
	 * Whether the contact is valid.
	 */
	std::optional<bool> valid = std::nullopt;

	/*
	 * Whether the contact is stable.
	 */
	std::optional<bool> stable = std::nullopt;

public:
	static std::optional<Contact<T>> find_in_frame(const usize index,
	                                               const std::vector<Contact<T>> &frame)
	{
		for (const Contact<T> &contact : frame) {
			if (contact.index != index)
				continue;

			return contact;
		}

		return std::nullopt;
	}
};

} // namespace iptsd::contacts

#endif // IPTSD_CONTACTS_CONTACT_HPP
