// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_DEVICE_HPP
#define IPTSD_IPTS_DEVICE_HPP

#include "data.hpp"
#include "descriptor.hpp"
#include "parser.hpp"

#include <common/error.hpp>
#include <common/types.hpp>
#include <hid/device.hpp>
#include <hid/report.hpp>

#include <gsl/gsl>

#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

namespace iptsd::ipts {
namespace impl {

enum class DeviceError : u8 {
	InvalidDevice,
	InvalidSetModeReport,
};

inline std::string format_as(DeviceError err)
{
	switch (err) {
	case DeviceError::InvalidDevice:
		return "ipts: {} is not an IPTS device!";
	case DeviceError::InvalidSetModeReport:
		return "ipts: The report for switching modes on {} is invalid!";
	default:
		return "ipts: Invalid error code!";
	}
}

} // namespace impl

enum class Mode : u8 {
	Singletouch = 0,
	Multitouch = 1,
};

class Device {
public:
	using Error = impl::DeviceError;

private:
	// The (platform specific) HID device interface
	std::shared_ptr<hid::Device> m_hid;

	// Support code for interfacing with the device through the HID descriptor
	Descriptor m_descriptor;

	// All reports of the HID descriptor that contain touch data
	std::vector<hid::Report> m_touch_data_reports;

public:
	Device(std::shared_ptr<hid::Device> hid)
		: m_hid {std::move(hid)},
		  m_descriptor {m_hid->descriptor()},
		  m_touch_data_reports {m_descriptor.find_touch_data_reports()}
	{
		// Check if the device can switch modes
		if (!m_descriptor.find_modesetting_report().has_value())
			throw common::Error<Error::InvalidDevice> {m_hid->name()};

		// Check if the device can send touch data.
		if (m_descriptor.find_touch_data_reports().empty())
			throw common::Error<Error::InvalidDevice> {m_hid->name()};
	};

	/*!
	 * The HID descriptor of the IPTS device.
	 */
	[[nodiscard]] const Descriptor &descriptor() const
	{
		return m_descriptor;
	}

	/*!
	 * Determines the required size for a buffer holding IPTS touch data.
	 *
	 * @return The size of the largest touch data report that IPTS can send.
	 */
	[[nodiscard]] usize buffer_size() const
	{
		u64 size = 0;

		for (const hid::Report &report : m_touch_data_reports)
			size = std::max(size, report.size());

		return casts::to<usize>(size / 8);
	}

	/*!
	 * Reads the IPTS device metadata from the metadata feature report.
	 *
	 * @return The metadata of the device, or null if the report is not supported.
	 */
	[[nodiscard]] std::optional<const Metadata> metadata() const
	{
		std::optional<Metadata> metadata = std::nullopt;

		const std::optional<hid::Report> report = m_descriptor.find_metadata_report();
		if (!report.has_value())
			return std::nullopt;

		const std::optional<u8> id = report->id();
		if (!id.has_value())
			return std::nullopt;

		std::vector<u8> buffer((report->size() / 8) + 1);
		buffer[0] = id.value();

		m_hid->get_feature(buffer);

		Parser parser {};
		parser.on_metadata = [&](const Metadata &m) { metadata = m; };
		parser.parse<u8>(buffer);

		return metadata;
	}

	/*!
	 * Changes the mode of the IPTS device.
	 *
	 * If the device is already in the requested mode, nothing will happen.
	 *
	 * @param[in] mode The new mode of operation.
	 */
	void set_mode(const Mode mode) const
	{
		const std::optional<hid::Report> report = m_descriptor.find_modesetting_report();
		if (!report.has_value())
			throw common::Error<Error::InvalidDevice> {m_hid->name()};

		const std::optional<u8> id = report->id();
		if (!id.has_value())
			throw common::Error<Error::InvalidSetModeReport> {m_hid->name()};

		std::array<u8, 2> buffer {id.value(), gsl::narrow<u8>(mode)};
		m_hid->set_feature(buffer);
	}

	/*!
	 * Checks whether a buffer contains IPTS touch data.
	 *
	 * @param[in] buffer The buffer that was read from the device.
	 * @return Whether the buffer contains touchscreen data.
	 */
	[[nodiscard]] bool is_touch_data(const gsl::span<u8> buffer) const
	{
		if (buffer.empty())
			return false;

		return std::any_of(
			m_touch_data_reports.cbegin(),
			m_touch_data_reports.cend(),
			[&](const hid::Report &report) { return report.id() == buffer[0]; });
	}
};

} // namespace iptsd::ipts

#endif // IPTSD_IPTS_DEVICE_HPP
