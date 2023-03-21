// SPDX-License-Identifier: GPL-2.0-or-later

#include "calibrate.hpp"

#include <common/signal.hpp>
#include <common/types.hpp>
#include <core/linux/device-runner.hpp>

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

namespace iptsd::apps::calibrate {

static int run(const gsl::span<char *> args)
{
	CLI::App app {"Utility for measuring your finger size and calibrating iptsd."};

	std::filesystem::path path {};
	app.add_option("DEVICE", path)
		->description("The hidraw device node of the touchscreen.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	// Create a calibration application that reads from a device.
	core::linux::DeviceRunner<Calibrate> calibrate {path};

	const auto _sigterm = common::signal<SIGTERM>([&](int) { calibrate.stop(); });
	const auto _sigint = common::signal<SIGINT>([&](int) { calibrate.stop(); });

	if (!calibrate.run())
		return EXIT_FAILURE;

	return 0;
}

} // namespace iptsd::apps::calibrate

int main(int argc, char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");
	const gsl::span<char *> args {argv, gsl::narrow<usize>(argc)};

	try {
		return iptsd::apps::calibrate::run(args);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
