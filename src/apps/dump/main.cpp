// SPDX-License-Identifier: GPL-2.0-or-later

#include "dump.hpp"

#include <common/types.hpp>
#include <core/linux/device-runner.hpp>
#include <core/linux/signal-handler.hpp>

#include <CLI/CLI.hpp>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>

namespace iptsd::apps::dump {
namespace {

int run(const int argc, const char **argv)
{
	CLI::App app {"Utility for saving raw reports from your touchscreen to a binary file."};

	std::filesystem::path path {};
	app.add_option("DEVICE", path)
		->description("The hidraw device node of the touchscreen.")
		->type_name("FILE")
		->required();

	std::filesystem::path output {};
	app.add_option("OUTPUT", output)
		->description("The file in which the data will be saved.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, argc, argv);

	// Create a dumping application that reads from a device.
	core::linux::DeviceRunner<Dump> dump {path, output};

	const auto _sigterm = core::linux::signal<SIGTERM>([&](int) { dump.stop(); });
	const auto _sigint = core::linux::signal<SIGINT>([&](int) { dump.stop(); });

	if (!dump.run())
		return EXIT_FAILURE;

	return 0;
}

} // namespace
} // namespace iptsd::apps::dump

int main(const int argc, const char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::apps::dump::run(argc, argv);
	} catch (const std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
