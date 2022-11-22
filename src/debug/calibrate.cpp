// SPDX-License-Identifier: GPL-2.0-or-later

#include <common/signal.hpp>
#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/finder.hpp>
#include <container/ops.hpp>
#include <ipts/device.hpp>
#include <ipts/parser.hpp>

#include <CLI/CLI.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <gsl/gsl>
#include <iostream>
#include <map>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace iptsd::debug::calibrate {

static std::vector<f64> size {};   // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static std::vector<f64> aspect {}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static void iptsd_calibrate_handle_input(const config::Config &config,
					 contacts::ContactFinder &finder, const ipts::Heatmap &data)
{
	// Make sure that all buffers have the correct size
	finder.resize(index2_t {data.dim.width, data.dim.height});

	// Normalize and invert the heatmap data.
	std::transform(data.data.begin(), data.data.end(), finder.data().begin(), [&](auto v) {
		f32 val = static_cast<f32>(v - data.dim.z_min) /
			  static_cast<f32>(data.dim.z_max - data.dim.z_min);

		return 1.0f - val;
	});

	// Search contacts
	const std::vector<contacts::Contact> &contacts = finder.search();

	// Calculate size and aspect of all stable contacts
	for (const auto &contact : contacts) {
		if (!contact.active || !contact.stable)
			continue;

		size.push_back(contact.major * std::hypot(config.width, config.height));
		aspect.push_back(contact.major / contact.minor);
	}

	if (size.size() == 0)
		return;

	std::sort(size.begin(), size.end());
	std::sort(aspect.begin(), aspect.end());

	f64 size_avg = container::ops::sum(size) / static_cast<f64>(size.size());
	f64 aspect_avg = container::ops::sum(aspect) / static_cast<f64>(aspect.size());

	// Determine 1st and 99th percentile
	f64 min_idx = std::max(gsl::narrow<f64>(size.size()) - 1, 0.0) * 0.01;
	f64 max_idx = std::max(gsl::narrow<f64>(size.size()) - 1, 0.0) * 0.99;

	f64 size_min = size[gsl::narrow<std::size_t>(std::round(min_idx))];
	f64 size_max = size[gsl::narrow<std::size_t>(std::round(max_idx))];

	f64 aspect_min = aspect[gsl::narrow<std::size_t>(std::round(min_idx))];
	f64 aspect_max = aspect[gsl::narrow<std::size_t>(std::round(max_idx))];

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
	CLI::App app {};
	std::filesystem::path path;

	app.add_option("DEVICE", path, "The hidraw device to read from.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	std::atomic_bool should_exit = false;

	const auto _sigterm = common::signal<SIGTERM>([&](int) { should_exit = true; });
	const auto _sigint = common::signal<SIGINT>([&](int) { should_exit = true; });

	ipts::Device device {path};

	auto meta = device.get_metadata();
	if (meta.has_value()) {
		auto &t = meta->transform;
		auto &u = meta->unknown.unknown;

		spdlog::info("Metadata:");
		spdlog::info("rows={}, columns={}", meta->size.rows, meta->size.columns);
		spdlog::info("width={}, height={}", meta->size.width, meta->size.height);
		spdlog::info("transform=[{},{},{},{},{},{}]", t.xx, t.yx, t.tx, t.xy, t.yy, t.ty);
		spdlog::info("unknown={}, [{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}]",
			     meta->unknown_byte, u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7],
			     u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]);
	}

	config::Config config {device.vendor(), device.product(), meta};

	// Check if a config was found
	if (config.width == 0 || config.height == 0)
		throw std::runtime_error("No display config for this device was found!");

	spdlog::info("Connected to device {:04X}:{:04X}", device.vendor(), device.product());
	spdlog::info("Samples: 0");
	spdlog::info("Size:    0.000 (Min: 0.000; Max: 0.000)");
	spdlog::info("Aspect:  0.000 (Min: 0.000; Max: 0.000)");

	contacts::ContactFinder finder {config.contacts()};

	ipts::Parser parser {};
	parser.on_heatmap = [&](const auto &data) {
		iptsd_calibrate_handle_input(config, finder, data);
	};

	// Get the buffer size from the HID descriptor
	std::size_t buffer_size = device.buffer_size();
	std::vector<u8> buffer(buffer_size);

	// Count errors, if we receive 50 continuous errors, chances are pretty good that
	// something is broken beyond repair and the program should exit.
	i32 errors = 0;

	// Enable multitouch mode
	device.set_mode(true);

	while (!should_exit) {
		if (errors >= 50) {
			spdlog::error("Encountered 50 continuous errors, aborting...");
			break;
		}

		try {
			ssize_t size = device.read(buffer);

			// Does this report contain touch data?
			if (!device.is_touch_data(buffer[0]))
				continue;

			parser.parse(gsl::span<u8>(buffer.data(), size));
		} catch (std::exception &e) {
			spdlog::warn(e.what());
			errors++;
			continue;
		}

		// Reset error count
		errors = 0;
	}

	spdlog::info("Stopping");

	// Disable multitouch mode
	device.set_mode(false);

	return 0;
}

} // namespace iptsd::debug::calibrate

int main(int argc, char *argv[])
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::debug::calibrate::main(gsl::span<char *>(argv, argc));
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
