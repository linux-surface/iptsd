// SPDX-License-Identifier: GPL-2.0-or-later

#include "visualize-png.hpp"

#include <common/signal.hpp>
#include <common/types.hpp>
#include <core/linux/file-runner.hpp>

#include <CLI/CLI.hpp>
#include <algorithm>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

namespace iptsd::apps::visualization::plot {

static int run(const gsl::span<char *> args)
{
	CLI::App app {"Utility for rendering captured touchscreen inputs to PNG frames."};

	std::filesystem::path path {};
	app.add_option("DATA", path)
		->description("A binary data file containing touch reports.")
		->type_name("FILE")
		->required();

	std::filesystem::path output {};
	app.add_option("OUTPUT", output)
		->description("The directory where the rendered frames are saved.")
		->type_name("DIR")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	// Create a plotting application that reads from a file.
	core::linux::FileRunner<VisualizePNG> visualize {path, output};

	const auto _sigterm = common::signal<SIGTERM>([&](int) { visualize.stop(); });
	const auto _sigint = common::signal<SIGINT>([&](int) { visualize.stop(); });

	if (!visualize.run())
		return EXIT_FAILURE;

	return 0;
}

} // namespace iptsd::apps::visualization::plot

int main(int argc, char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");
	const gsl::span<char *> args {argv, gsl::narrow<usize>(argc)};

	try {
		return iptsd::apps::visualization::plot::run(args);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
