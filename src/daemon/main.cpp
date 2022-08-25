// SPDX-License-Identifier: GPL-2.0-or-later

#include "context.hpp"
#include "devices.hpp"
#include "dft.hpp"
#include "stylus.hpp"
#include "touch.hpp"

#include <common/signal.hpp>
#include <common/types.hpp>
#include <ipts/device.hpp>
#include <ipts/parser.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <thread>

using namespace std::chrono;

namespace iptsd::daemon {

static int main(gsl::span<char *> args)
{
	if (args.size() < 2)
		throw std::runtime_error("You need to specify the hidraw device!");

	std::atomic_bool should_exit = false;

	auto const _sigterm = common::signal<SIGTERM>([&](int) { should_exit = true; });
	auto const _sigint = common::signal<SIGINT>([&](int) { should_exit = true; });

	ipts::Device device {args[1]};
	Context ctx {device.vendor(), device.product()};

	spdlog::info("Connected to device {:04X}:{:04X}", device.vendor(), device.product());

	ipts::Parser parser {};
	parser.on_stylus = [&](const auto &data) { iptsd_stylus_input(ctx, data); };
	parser.on_heatmap = [&](const auto &data) { iptsd_touch_input(ctx, data); };
	parser.on_dft = [&](const auto &dft, auto &stylus) { iptsd_dft_input(ctx, dft, stylus); };

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

	return EXIT_FAILURE;
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
