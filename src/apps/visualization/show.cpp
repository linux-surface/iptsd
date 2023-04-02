// SPDX-License-Identifier: GPL-2.0-or-later

#include "visualize-sdl.hpp"

#include <common/types.hpp>
#include <core/linux/device-runner.hpp>
#include <core/linux/signal-handler.hpp>

#include <CLI/CLI.hpp>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>

namespace iptsd::apps::visualization::show {

static int run(const gsl::span<char *> args)
{
	CLI::App app {"Utility for rendering touchscreen inputs in real time."};

	std::filesystem::path path {};
	app.add_option("DEVICE", path)
		->description("The hidraw device node of the touchscreen.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	// Create a plotting application that reads from a file.
	core::linux::DeviceRunner<VisualizeSDL> visualize {path};

	const auto _sigterm = core::linux::signal<SIGTERM>([&](int) { visualize.stop(); });
	const auto _sigint = core::linux::signal<SIGINT>([&](int) { visualize.stop(); });

	if (!visualize.run())
		return EXIT_FAILURE;

	return 0;
}

} // namespace iptsd::apps::visualization::show

int main(int argc, char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");
	const gsl::span<char *> args {argv, gsl::narrow<usize>(argc)};

	try {
		return iptsd::apps::visualization::show::run(args);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
