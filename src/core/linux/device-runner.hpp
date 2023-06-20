// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_DEVICE_RUNNER_HPP
#define IPTSD_CORE_LINUX_DEVICE_RUNNER_HPP

#include "config-loader.hpp"
#include "hidraw-device.hpp"

#include <common/casts.hpp>
#include <common/chrono.hpp>
#include <core/generic/application.hpp>
#include <ipts/data.hpp>
#include <ipts/device.hpp>

#include <spdlog/spdlog.h>

#include <atomic>
#include <filesystem>
#include <memory>
#include <thread>
#include <type_traits>
#include <vector>

namespace iptsd::core::linux {

template <class T>
class DeviceRunner {
private:
	static_assert(std::is_base_of_v<Application, T>);

private:
	// The hidraw device serving as the source of data.
	std::shared_ptr<HidrawDevice> m_device;

	// The IPTS touchscreen interface
	ipts::Device m_ipts;

	// Whether the loop for reading from the device should stop.
	std::atomic_bool m_should_stop = false;

	// The target buffer for reading HID reports.
	std::vector<u8> m_buffer {};

	/*
	 * deferred initialization
	 */

	// The application that is being executed.
	std::optional<T> m_application = std::nullopt;

public:
	template <class... Args>
	DeviceRunner(const std::filesystem::path &path, Args... args)
		: m_device {std::make_shared<HidrawDevice>(path)}
		, m_ipts {m_device}
	{
		DeviceInfo info {};
		info.vendor = m_device->vendor();
		info.product = m_device->product();
		info.buffer_size = m_ipts.buffer_size();

		const std::optional<const ipts::Metadata> meta = m_ipts.metadata();

		const ConfigLoader loader {info, meta};
		m_application.emplace(loader.config(), info, meta, args...);

		m_buffer.resize(casts::to<usize>(info.buffer_size));

		const u16 vendor = info.vendor;
		const u16 product = info.product;

		spdlog::info("Connected to device {:04X}:{:04X}", vendor, product);
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
		return m_application.value();
	}

	/*!
	 * Stops the loop that reads from the device.
	 *
	 * This function is designed to be called from a signal handler (e.g. for Ctrl-C).
	 */
	void stop()
	{
		m_should_stop = true;
	}

	/*!
	 * Starts reading from the device in an endless loop.
	 *
	 * Touch data that is read will be passed to the application that is being executed.
	 */
	bool run()
	{
		if (!m_application.has_value())
			throw std::runtime_error("Init error: Application is null");

		// Enable multitouch mode
		m_ipts.set_mode(ipts::Mode::Multitouch);

		// Signal the application that the data flow has started.
		m_application->on_start();

		usize errors = 0;

		while (!m_should_stop) {
			if (errors >= 50) {
				spdlog::error("Encountered 50 continuous errors, aborting...");
				break;
			}

			try {
				const isize size = m_device->read(m_buffer);

				// Does this report contain touch data?
				if (!m_ipts.is_touch_data(m_buffer))
					continue;

				m_application->process(
					gsl::span<u8>(m_buffer.data(), casts::to_unsigned(size)));
			} catch (std::exception &e) {
				spdlog::warn(e.what());

				// Sleep for a moment to let the device get back into normal state.
				std::this_thread::sleep_for(100ms);

				errors++;
				continue;
			}

			// Reset error count.
			errors = 0;
		}

		spdlog::info("Stopping");

		// Signal the application that the data flow has stopped.
		m_application->on_stop();

		try {
			// Disable multitouch mode
			m_ipts.set_mode(ipts::Mode::Singletouch);
		} catch (std::exception &e) {
			spdlog::error(e.what());
		}

		return m_should_stop;
	}
};

} // namespace iptsd::core::linux

#endif // IPTSD_CORE_LINUX_DEVICE_RUNNER_HPP
