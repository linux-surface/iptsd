// SPDX-License-Identifier: GPL-2.0-or-later

#include "visualize-sdl.hpp"

#include <common/signal.hpp>
#include <common/types.hpp>
#include <core/linux/device-runner.hpp>

#include <CLI/CLI.hpp>
#include <algorithm>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

namespace iptsd::apps::visualization::show {

static int main(gsl::span<char *> args)
{
	CLI::App app {};
	std::filesystem::path path;

	app.add_option("DEVICE", path, "The hidraw device to read from.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	// Create a plotting application that reads from a file.
	core::linux::DeviceRunner<VisualizeSDL> visualize {path};

	auto const _sigterm = common::signal<SIGTERM>([&](int) { visualize.stop(); });
	auto const _sigint = common::signal<SIGINT>([&](int) { visualize.stop(); });

	if (!visualize.run())
		return EXIT_FAILURE;

	return 0;
}

} // namespace iptsd::apps::visualization::show

int main(int argc, char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");
	gsl::span<char *> args {argv, gsl::narrow<usize>(argc)};

	try {
		return iptsd::apps::visualization::show::main(args);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
