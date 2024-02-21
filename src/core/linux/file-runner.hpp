// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_FILE_RUNNER_HPP
#define IPTSD_CORE_LINUX_FILE_RUNNER_HPP

#include "config-loader.hpp"

#include <common/casts.hpp>
#include <common/reader.hpp>
#include <core/generic/application.hpp>
#include <ipts/data.hpp>

#include <spdlog/spdlog.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <type_traits>
#include <vector>

namespace iptsd::core::linux {

template <class T>
class FileRunner {
private:
	static_assert(std::is_base_of_v<Application, T>);

private:
	// The contents of the file.
	std::vector<u8> m_file {};

	// Information about the device that produced the data.
	DeviceInfo m_info {};

	// Whether the loop for reading from the file should stop.
	std::atomic_bool m_should_stop = false;

	/*
	 * deferred initialization
	 */

	// The application that is being executed.
	std::optional<T> m_application = std::nullopt;
	std::optional<Reader> m_reader = std::nullopt;

public:
	template <class... Args>
	FileRunner(const std::filesystem::path &path, Args... args)
	{
		std::ifstream ifs {};
		ifs.open(path, std::ios::in | std::ios::binary);

		std::noskipws(ifs);
		m_file = std::vector<u8> {std::istream_iterator<u8>(ifs),
		                          std::istream_iterator<u8>()};

		m_reader = Reader {m_file};
		m_info = m_reader->read<DeviceInfo>();

		std::optional<ipts::Metadata> meta = std::nullopt;

		const auto has_meta = m_reader->read<u8>();
		if (has_meta)
			meta = m_reader->read<ipts::Metadata>();

		const ConfigLoader loader {m_info, meta};
		m_application.emplace(loader.config(), m_info, meta, args...);

		const u16 vendor = m_info.vendor;
		const u16 product = m_info.product;

		spdlog::info("Loaded from device {:04X}:{:04X}", vendor, product);
	}

	/*!
	 * The application instance that is being run.
	 *
	 * Can be used to access collected data or to reset a state.
	 *
	 * @return A reference to the application instance that is being run.
	 */
	T &application()
	{
		if (!m_application.has_value())
			throw common::Error<Error::RunnerInitError> {};

		return m_application.value();
	}

	/*!
	 * Stops the loop that reads from the file.
	 *
	 * This function is designed to be called from a signal handler (e.g. for Ctrl-C).
	 */
	void stop()
	{
		m_should_stop = true;
	}

	/*!
	 * Starts reading from the file until no data is left.
	 *
	 * Touch data that is read will be passed to the application that is being executed.
	 * This function can safely be called multiple times in a row.
	 */
	bool run()
	{
		if (!m_application.has_value() || !m_reader.has_value())
			throw common::Error<Error::RunnerInitError> {};

		Reader local = m_reader.value();

		// Signal the application that the data flow has started.
		m_application->on_start();

		while (!m_should_stop && local.size() > 0) {
			try {
				/*
				 * Abort if there is not enough data left.
				 */
				if (local.size() < (sizeof(u64) + m_info.buffer_size))
					break;

				const auto size = local.read<u64>();

				/*
				 * This is an error baked into the format.
				 * The writer should simply write as many bytes as it just received,
				 * instead of writing the entire buffer all the time.
				 */
				Reader buffer = local.sub(casts::to<usize>(m_info.buffer_size));

				m_application->process(buffer.subspan<u8>(casts::to<usize>(size)));
			} catch (const std::exception &e) {
				spdlog::warn(e.what());
			}
		}

		if (!m_should_stop && local.size() > 0)
			spdlog::warn("Leftover data at end of input");

		// Signal the application that the data flow has stopped.
		m_application->on_stop();

		return m_should_stop;
	}
};

} // namespace iptsd::core::linux

#endif // IPTSD_CORE_LINUX_FILE_RUNNER_HPP
