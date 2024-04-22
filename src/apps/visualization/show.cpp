// SPDX-License-Identifier: GPL-2.0-or-later

#include "visualize-sdl.hpp"

#include <common/types.hpp>
#include <core/linux/device/hidraw.hpp>
#include <core/linux/runner.hpp>
#include <core/linux/signal-handler.hpp>

#include <CLI/CLI.hpp>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>

namespace iptsd::apps::visualization::show {
namespace {

int run(const int argc, const char **argv)
{
	CLI::App app {"Utility for rendering touchscreen inputs in real time"};

	std::filesystem::path path {};
	app.add_option("DEVICE", path)
		->description("The hidraw device node of the touchscreen")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, argc, argv);

	// Create a plotting application that reads from a file.
	core::linux::Runner<VisualizeSDL, core::linux::device::Hidraw> visualize {path};

	const auto _sigterm = core::linux::signal<SIGTERM>([&](int) { visualize.stop(); });
	const auto _sigint = core::linux::signal<SIGINT>([&](int) { visualize.stop(); });

	if (!visualize.run())
		return EXIT_FAILURE;

	return 0;
}

} // namespace
} // namespace iptsd::apps::visualization::show

int main(const int argc, const char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::apps::visualization::show::run(argc, argv);
	} catch (const std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
