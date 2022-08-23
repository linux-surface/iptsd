// SPDX-License-Identifier: GPL-2.0-or-later

#include <common/signal.hpp>
#include <contacts/finder.hpp>
#include <container/ops.hpp>
#include <drm/device.hpp>
#include <ipts/device.hpp>
#include <ipts/parser.hpp>
#include <ipts/protocol.hpp>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>
#include <cmath>
#include <gsl/gsl>
#include <iostream>
#include <map>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace iptsd::debug::finger {

static std::vector<f64> size {};   // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static std::vector<f64> aspect {}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static void iptsd_finger_handle_input(const drm::Device &display, contacts::ContactFinder &finder,
				      const ipts::Heatmap &data)
{
	// Make sure that all buffers have the correct size
	finder.resize(index2_t {data.dim.width, data.dim.height});

	// Normalize and invert the heatmap data.
	std::transform(data.data.begin(), data.data.end(), finder.data().begin(), [&](auto v) {
		f32 val = static_cast<f32>(v - data.dim.z_min) /
			  static_cast<f32>(data.dim.z_max - data.dim.z_min);

		return 1.0f - val;
	});

	// Search for a contact
	const contacts::Contact &contact = finder.search()[0];

	// If there was no contact, return
	if (!contact.active)
		return;

	// Calculate size and aspect
	size.push_back(contact.major * std::hypot(display.width, display.height));
	aspect.push_back(contact.major / contact.minor);

	f64 size_avg = container::ops::sum(size) / static_cast<f64>(size.size());
	auto [size_min, size_max] = container::ops::minmax(size);

	f64 aspect_avg = container::ops::sum(aspect) / static_cast<f64>(aspect.size());
	auto [aspect_min, aspect_max] = container::ops::minmax(aspect);

	// Reset console output
	std::cout << "\033[A"; // Move cursor up one line
	std::cout << "\33[2K"; // Erase current line
	std::cout << "\033[A"; // Move cursor up one line
	std::cout << "\33[2K"; // Erase current line
	std::cout << "\033[A"; // Move cursor up one line
	std::cout << "\33[2K"; // Erase current line
	std::cout << "\r";     // Move cursor to the left

	spdlog::info("Samples: {}", size.size());
	spdlog::info("Size:    {:.3f} (Min: {:.3f}; Max: {:.3f})", size_avg, size_min, size_max);
	spdlog::info("Aspect:  {:.3f} (Min: {:.3f}; Max: {:.3f})", aspect_avg, aspect_min,
		     aspect_max);
}

static int main(gsl::span<char *> args)
{
	std::filesystem::path path;
	contacts::BlobDetection mode = contacts::BlobDetection::BASIC;

	CLI::App app {"Measure the size of your fingers."};

	std::map<std::string, contacts::BlobDetection> map {
		{"basic", contacts::BlobDetection::BASIC},
		{"advanced", contacts::BlobDetection::ADVANCED},
	};

	app.add_option("-d,--device", path, "The IPTS hidraw device.")
		->type_name("FILE")
		->required();

	app.add_option("-m,--mode", mode, "The blob detection mode to use.")
		->default_str("basic")
		->transform(CLI::CheckedTransformer {map, CLI::ignore_case});

	CLI11_PARSE(app, args.size(), args.data());

	std::atomic_bool should_exit {false};
	const auto _sigterm = common::signal<SIGTERM>([&](int) { should_exit = true; });
	const auto _sigint = common::signal<SIGINT>([&](int) { should_exit = true; });

	drm::Device display {};
	ipts::Device device {path};

	spdlog::info("Connected to device {:04X}:{:04X}", device.vendor(), device.product());
	spdlog::info("Samples: 0");
	spdlog::info("Size:    0.000 (Min: 0.000; Max: 0.000)");
	spdlog::info("Aspect:  0.000 (Min: 0.000; Max: 0.000)");

	contacts::Config cfg {};
	cfg.max_contacts = 1;
	cfg.width = display.width;
	cfg.height = display.height;
	cfg.mode = mode;

	contacts::ContactFinder finder {cfg};

	ipts::Parser parser {};
	parser.on_heatmap = [&](const auto &data) {
		iptsd_finger_handle_input(display, finder, data);
	};

	// Get the buffer size from the HID descriptor
	std::size_t buffer_size = device.buffer_size();
	std::vector<u8> buffer(buffer_size);

	// Enable multitouch mode
	device.set_mode(true);

	while (!should_exit) {
		try {
			ssize_t size = device.read(buffer);

			// Does this report contain touch data?
			if (!device.is_touch_data(buffer[0]))
				continue;

			parser.parse(gsl::span<u8>(buffer.data(), size));
		} catch (std::exception &e) {
			spdlog::error(e.what());
			break;
		}
	}

	spdlog::info("Stopping");

	// Disable multitouch mode
	device.set_mode(false);

	return 0;
}

} // namespace iptsd::debug::finger

int main(int argc, char *argv[])
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::debug::finger::main(gsl::span<char *>(argv, argc));
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
