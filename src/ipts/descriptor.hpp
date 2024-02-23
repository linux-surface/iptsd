// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_DESCRIPTOR_HPP
#define IPTSD_IPTS_DESCRIPTOR_HPP

#include "protocol/descriptor.hpp"

#include <common/types.hpp>
#include <hid/descriptor.hpp>
#include <hid/report.hpp>

#include <optional>
#include <utility>
#include <vector>

namespace iptsd::ipts {

class Descriptor {
private:
	hid::Descriptor m_hid_descriptor;

public:
	Descriptor(hid::Descriptor desc) : m_hid_descriptor {std::move(desc)} {};

	/*!
	 * Searches for all reports that contain touch data in the HID descriptor.
	 *
	 * @return A list of reports containing IPTS touch data.
	 */
	[[nodiscard]] std::vector<hid::Report> find_touch_data_reports() const
	{
		return m_hid_descriptor.find_reports(protocol::descriptor::is_touch_data);
	}

	/*!
	 * Searches for the modesetting report in the HID descriptor.
	 *
	 * The modesetting report is a feature report for changing the mode of the device from
	 * singletouch to multitouch and vice versa.
	 *
	 * @return The HID report for modesetting if it exists, null otherwise.
	 */
	[[nodiscard]] std::optional<hid::Report> find_modesetting_report() const
	{
		return m_hid_descriptor.find_report(protocol::descriptor::is_set_mode);
	}

	/*!
	 * Searches for the metadata report in the HID descriptor.
	 *
	 * The metadata report is a feature report that returns data about the device, such as
	 * the screen size or the orientation of the touch data.
	 *
	 * @return The HID report for fetching metadata if it exists, null otherwise.
	 */
	[[nodiscard]] std::optional<hid::Report> find_metadata_report() const
	{
		return m_hid_descriptor.find_report(protocol::descriptor::is_metadata);
	}

	/*!
	 * Whether the HID descriptor indicates that this device is a touchscreen.
	 */
	[[nodiscard]] bool is_touchscreen() const
	{
		return m_hid_descriptor.has_collection(protocol::descriptor::is_touchscreen);
	}

	/*!
	 * Whether the HID descriptor indicates that this device is a touchpad.
	 */
	[[nodiscard]] bool is_touchpad() const
	{
		return m_hid_descriptor.has_collection(protocol::descriptor::is_touchpad);
	}
};

} // namespace iptsd::ipts

#endif // IPTSD_IPTS_DESCRIPTOR_HPP
