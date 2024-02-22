// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_DEVICE_CAPTURE_HPP
#define IPTSD_CORE_LINUX_DEVICE_CAPTURE_HPP

#include "hidraw.hpp"

#include <common/casts.hpp>
#include <common/chrono.hpp>
#include <common/file.hpp>
#include <common/types.hpp>
#include <hid/device.hpp>
#include <hid/parser.hpp>
#include <hid/report.hpp>
#include <ipts/parser.hpp>

#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include <linux/hidraw.h>

#include <filesystem>
#include <fstream>

namespace iptsd::core::linux::device {

class Capture : public Hidraw {
private:
	using clock = chrono::system_clock;

private:
	std::ofstream m_writer {};

public:
	Capture(const std::filesystem::path &path) : Hidraw(path)
	{
		const clock::duration now = clock::now().time_since_epoch();
		const usize unix = chrono::duration_cast<seconds<usize>>(now).count();

		const u16 vendor = this->vendor();
		const u16 product = this->product();

		const std::filesystem::path outpath =
			std::filesystem::temp_directory_path() /
			fmt::format("iptsd_{:04X}_{:04X}_{}.bin", vendor, product, unix);

		m_writer.exceptions(std::ios::badbit | std::ios::failbit);
		m_writer.open(outpath, std::ios::out | std::ios::binary);

		spdlog::info("Capturing HID traffic to {}", outpath.c_str());

		common::write_to_stream(m_writer, m_devinfo);
		common::write_to_stream(m_writer, m_desc.size);
		common::write_to_stream(m_writer, gsl::span<u8> {&m_desc.value[0], m_desc.size});
	}

	~Capture() override
	{
		m_writer.close();
	}

	/*!
	 * Reads a report from the HID device.
	 *
	 * @param[in] buffer The target storage for the report.
	 * @return The size of the report that was read in bytes.
	 */
	usize read(gsl::span<u8> buffer) override
	{
		const usize size = Hidraw::read(buffer);

		common::write_to_stream(m_writer, casts::to<u64>(size));
		common::write_to_stream(m_writer, buffer.first(size));

		return size;
	}

	/*!
	 * Gets the data of a HID feature report.
	 *
	 * @param[in] report The report ID to get, followed by enough space to fit the data.
	 */
	void get_feature(gsl::span<u8> report) override
	{
		Hidraw::get_feature(report);

		common::write_to_stream(m_writer, casts::to<u64>(report.size()));
		common::write_to_stream(m_writer, report);
	}
};

} // namespace iptsd::core::linux::device

#endif // IPTSD_CORE_LINUX_DEVICE_CAPTURE_HPP
