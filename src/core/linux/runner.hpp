// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_DEVICE_RUNNER_HPP
#define IPTSD_CORE_LINUX_DEVICE_RUNNER_HPP

#include "config-loader.hpp"
#include "device/errors.hpp"
#include "errors.hpp"

#include <common/casts.hpp>
#include <common/chrono.hpp>
#include <common/error.hpp>
#include <core/generic/application.hpp>
#include <ipts/device.hpp>

#include <spdlog/spdlog.h>

#include <atomic>
#include <filesystem>
#include <memory>
#include <thread>
#include <type_traits>
#include <vector>

namespace iptsd::core::linux {

/*!
 * The application runner is responsible for connecting a generic application with the
 * hardware and platform specific implementation details.
 *
 * @tparam App The application type that is being run.
 * @tparam Device The type that implements the HID data source.
 */
template <class App, class Device>
class Runner {
private:
	static_assert(std::is_base_of_v<Application, App>);
	static_assert(std::is_base_of_v<hid::Device, Device>);

private:
	// The hidraw device serving as the source of data.
	std::shared_ptr<hid::Device> m_device;

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
	std::optional<App> m_application = std::nullopt;

public:
	template <class... Args>
	Runner(const std::filesystem::path &path, Args... args)
		: m_device {std::make_shared<Device>(path)},
		  m_ipts {m_device}
	{
		DeviceInfo info {};
		info.vendor = m_device->vendor();
		info.product = m_device->product();
		info.type = m_ipts.type();
		info.meta = m_ipts.metadata();

		const ConfigLoader loader {info};
		m_application.emplace(loader.config(), info, args...);

		m_buffer.resize(m_ipts.buffer_size());

		const u16 vendor = info.vendor;
		const u16 product = info.product;

		spdlog::info("Connected to device {:04X}:{:04X}", vendor, product);

		switch (info.type) {
		case ipts::Device::Type::Touchscreen:
			spdlog::info("Running in Touchscreen mode");
			break;
		case ipts::Device::Type::Touchpad:
			spdlog::info("Running in Touchpad mode");
			break;
		}
	}

	/*!
	 * The application instance that is being run.
	 *
	 * Can be used to access collected data or to reset a state.
	 *
	 * @return A reference to the application instance that is being run.
	 */
	App &application()
	{
		if (!m_application.has_value())
			throw common::Error<Error::RunnerInitError> {};

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
	 * Starts reading from the device, until the device signals that no more data is available.
	 *
	 * Touch data that is read will be passed to the application that is being executed.
	 * Depending on the HID data source, this method can be called multiple times in a row.
	 */
	bool run()
	{
		if (!m_application.has_value())
			throw common::Error<Error::RunnerInitError> {};

		// Enable multitouch mode
		m_ipts.set_mode(ipts::Device::Mode::Multitouch);

		// Signal the application that the data flow has started.
		m_application->on_start();

		usize errors = 0;

		while (!m_should_stop) {
			if (errors >= 50) {
				spdlog::error("Encountered 50 continuous errors, aborting...");
				break;
			}

			try {
				const usize size = m_device->read(m_buffer);
				const gsl::span<u8> data {m_buffer.data(), size};

				// Does this report contain touch data?
				if (!m_ipts.is_touch_data(m_buffer))
					continue;

				m_application->process(data);
			} catch (const common::Error<device::Error::EndOfData> & /* unused */) {
				break;
			} catch (const std::exception &e) {
				spdlog::warn(e.what());

				// Sleep for a moment to let the device get back into normal state.
				std::this_thread::sleep_for(100ms);

				errors++;
				continue;
			}

			// Reset error count.
			errors = 0;
		}

		// Signal the application that the data flow has stopped.
		m_application->on_stop();

		try {
			// Disable multitouch mode
			m_ipts.set_mode(ipts::Device::Mode::Singletouch);
		} catch (const std::exception &e) {
			spdlog::error(e.what());
		}

		return m_should_stop;
	}
};

} // namespace iptsd::core::linux

#endif // IPTSD_CORE_LINUX_DEVICE_RUNNER_HPP
