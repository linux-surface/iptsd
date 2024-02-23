// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_HID_REPORT_HPP
#define IPTSD_HID_REPORT_HPP

#include "field.hpp"

#include <common/types.hpp>

#include <optional>
#include <vector>

namespace iptsd::hid {

/*!
 * A report is data that is sent between the host and a device.
 *
 * The structure of this data, as well as how it should be interpreted, are defined by the
 * report descriptor using an 8 bit Report ID.
 */
class Report {
public:
	enum class Type : u8 {
		//! An input report is data that is sent from the device to device.
		//! It can contain everything from keypresses to mouse positions.
		Input,

		//! An output report is data that is sent from the host to the device.
		//! For example it could be used to set the state of one of multiple LEDs.
		Output,

		//! Feature reports are a configuration option on the device.
		//! The host can either read the current value, or set a new one.
		Feature,
	};

public:
	/*!
	 * The type of the report indicates the direction in which data is flowing.
	 *
	 * See also @ref Report::Type.
	 */
	Type type {};

	/*!
	 * The Report ID is used to identify the report when communicating with the device.
	 * It is stored as a single byte in front of the report data.
	 * However, the report ID is optional.
	 */
	std::optional<u8> report_id = std::nullopt;

	/*!
	 * A HID report groups together multiple fields.
	 * Every field has its own size and purpose (defined by the Usage tag).
	 */
	std::vector<Field> fields {};

public:
	/*!
	 * The combined size of all report fields, in bits.
	 */
	[[nodiscard]] usize bits() const
	{
		usize sum = 0;

		for (const Field &field : this->fields)
			sum += field.bits();

		return sum;
	}

	/*!
	 * The combined size of all report fields, in bytes.
	 */
	[[nodiscard]] usize bytes() const
	{
		const f64 frac = casts::to<f64>(this->bits()) / 8;
		return casts::to<usize>(std::ceil(frac));
	}

	/*!
	 * Searches for a field with the given Usage tag.
	 *
	 * @param[in] page The Usage Page to look for (upper 16 bits of the Usage tag).
	 * @param[in] id The Usage ID to look for (lower 16 bits of the Usage tag).
	 * @return Whether a field with a matching Usage tag was found.
	 */
	[[nodiscard]] bool has_usage(const u16 page, const u16 id) const
	{
		return std::any_of(this->fields.begin(), this->fields.end(), [&](const Field &f) {
			return f.has_usage(page, id);
		});
	}
};

} // namespace iptsd::hid

#endif // IPTSD_HID_REPORT_HPP
