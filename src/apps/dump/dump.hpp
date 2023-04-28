// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_DUMP_DUMP_HPP
#define IPTSD_APPS_DUMP_DUMP_HPP

#include <common/casts.hpp>
#include <common/types.hpp>
#include <core/generic/application.hpp>
#include <core/generic/config.hpp>
#include <core/generic/device.hpp>
#include <ipts/data.hpp>

#include <gsl/gsl>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <utility>

namespace iptsd::apps::dump {

class Dump : public core::Application {
private:
	std::filesystem::path m_out;
	std::ofstream m_writer {};

public:
	Dump(const core::Config &config,
	     const core::DeviceInfo &info,
	     std::optional<const ipts::Metadata> metadata,
	     std::filesystem::path output)
		: core::Application(config, info, metadata)
		, m_out {std::move(output)} {};

	void on_start() override
	{
		if (m_out.empty())
			return;

		m_writer.exceptions(std::ios::badbit | std::ios::failbit);
		m_writer.open(m_out, std::ios::out | std::ios::binary);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		m_writer.write(reinterpret_cast<char *>(&m_info), sizeof(m_info));

		const char has_meta = m_metadata.has_value() ? 1 : 0;
		m_writer.write(&has_meta, sizeof(has_meta));

		if (m_metadata.has_value()) {
			const ipts::Metadata m = m_metadata.value();

			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			m_writer.write(reinterpret_cast<const char *>(&m), sizeof(m));
		}
	}

	void on_data(const gsl::span<u8> data) override
	{
		if (m_out.empty())
			return;

		const u64 size = casts::to<u64>(data.size());

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		m_writer.write(reinterpret_cast<const char *>(&size), sizeof(size));

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		m_writer.write(reinterpret_cast<char *>(data.data()),
			       casts::to<std::streamsize>(size));

		// Pad the data with zeros, so that we always write a full buffer.
		std::fill_n(std::ostream_iterator<u8>(m_writer), m_info.buffer_size - size, '\0');
	}
};

} // namespace iptsd::apps::dump

#endif // IPTSD_APPS_DUMP_DUMP_HPP
