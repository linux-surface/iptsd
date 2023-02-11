// SPDX-License-Identifier: GPL-2.0-or-later

#include "context.hpp"
#include "devices.hpp"
#include "dft.hpp"
#include "stylus.hpp"
#include "touch.hpp"

#include <common/signal.hpp>
#include <common/types.hpp>
#include <config/config.hpp>
#include <ipts/device.hpp>
#include <ipts/parser.hpp>

#include <CLI/CLI.hpp>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <thread>

using namespace std::chrono;

namespace iptsd::daemon {

static int main(gsl::span<char *> args)
{
	CLI::App app {};
	std::filesystem::path path;

	app.add_option("DEVICE", path, "The hidraw device to read from.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	std::atomic_bool should_exit = false;

	auto const _sigterm = common::signal<SIGTERM>([&](int) { should_exit = true; });
	auto const _sigint = common::signal<SIGINT>([&](int) { should_exit = true; });

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

	Context ctx {config, meta};
	spdlog::info("Connected to device {:04X}:{:04X}", device.vendor(), device.product());

	ipts::Parser parser {};

	if (!config.touch_disable)
		parser.on_heatmap = [&](const auto &data) { iptsd_touch_input(ctx, data); };
	else
		spdlog::warn("Touchscreen is disabled!");

	if (!config.stylus_disable) {
		parser.on_stylus = [&](const auto &data) { iptsd_stylus_input(ctx, data); };
		parser.on_dft = [&](const auto &dft, auto &stylus) {
			iptsd_dft_input(ctx, dft, stylus);
		};
	} else {
		spdlog::warn("Stylus is disabled!");
	}

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

			// Sleep for 100ms to allow the device to get back to normal state
			std::this_thread::sleep_for(100ms);

			continue;
		}

		// Reset error count
		errors = 0;
	}

	spdlog::info("Stopping");

	try {
		// Disable multitouch mode
		device.set_mode(false);
	} catch (std::exception &e) {
		spdlog::error(e.what());
	}

	// If iptsd was stopped from outside, return no error
	if (!should_exit)
		return EXIT_FAILURE;

	return 0;
}

} // namespace iptsd::daemon

int main(int argc, char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::daemon::main(gsl::span<char *>(argv, argc));
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
