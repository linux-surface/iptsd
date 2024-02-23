// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_FIELD_HPP
#define IPTSD_HID_FIELD_HPP

#include <common/casts.hpp>
#include <common/types.hpp>

#include <optional>

namespace iptsd::hid {

/*!
 * A field is a discrete section of data within a report.
 *
 * It is similar to a field in a C structure, defining the size and type of the data inside,
 * as well as how many times the data is repeated.
 */
class Field {
public:
	/*!
	 * The size (in bits) of one element of the field.
	 * In a C structure this would be equivalent to the type (in combination with the usage).
	 */
	u32 size = 0;

	/*!
	 * How many elements the field declares.
	 * In a C structure this would be equivalent to declaring an array.
	 */
	u32 count = 0;

	/*!
	 * The Usage tag of this field.
	 *
	 * The Usage tag is a combination of the Usage Page in the upper 16 bits and the Usage ID
	 * in the lower 16 bits. It defines how the data in the field should be interpreted.
	 */
	u32 usage = 0;

	std::optional<u32> usage_min = std::nullopt;
	std::optional<u32> usage_max = std::nullopt;

	u32 logical_min = 0;
	u32 logical_max = 0;

	std::optional<u32> physical_min = std::nullopt;
	std::optional<u32> physical_max = std::nullopt;

	std::optional<u32> unit = std::nullopt;
	std::optional<u32> unit_exp = std::nullopt;

public:
	/*!
	 * The size of the field, in bits.
	 */
	[[nodiscard]] usize bits() const
	{
		return casts::to<usize>(this->count) * this->size;
	}

	/*!
	 * The size of the field, in bytes.
	 */
	[[nodiscard]] usize bytes() const
	{
		const f64 frac = casts::to<f64>(this->bits()) / 8;
		return casts::to<usize>(std::ceil(frac));
	}

	/*!
	 * Checks if a Usage applies to this field.
	 *
	 * @param page The Usage Page to check against (upper 16 bit of the Usage tag).
	 * @param id The Usage ID to check against (lower 16 bit of the Usage tag).
	 * @return Whether the field is marked with the given Usage.
	 */
	[[nodiscard]] bool has_usage(const u16 page, const u16 id) const
	{
		const u32 combined = (casts::to<u32>(page) << 16) | id;

		if (this->usage == combined)
			return true;

		if (!this->usage_min.has_value() || !this->usage_max.has_value())
			return false;

		return this->usage_min <= combined && combined <= this->usage_max;
	}
};

}; // namespace iptsd::hid

#endif // IPTSD_HID_FIELD_HPP
