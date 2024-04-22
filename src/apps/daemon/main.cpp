// SPDX-License-Identifier: GPL-2.0-or-later

#include "daemon.hpp"

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

namespace iptsd::apps::daemon {
namespace {

int run(const int argc, const char **argv)
{
	CLI::App app {"Daemon to translate touchscreen inputs to Linux input events"};

	std::filesystem::path path {};
	app.add_option("DEVICE", path)
		->description("The hidraw device node of the touchscreen")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, argc, argv);

	// Create a daemon application that reads from a device.
	core::linux::Runner<Daemon, core::linux::device::Hidraw> daemon {path};

	const auto _sigterm = core::linux::signal<SIGTERM>([&](int) { daemon.stop(); });
	const auto _sigint = core::linux::signal<SIGINT>([&](int) { daemon.stop(); });

	if (!daemon.run())
		return EXIT_FAILURE;

	return 0;
}

} // namespace
} // namespace iptsd::apps::daemon

int main(const int argc, const char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::apps::daemon::run(argc, argv);
	} catch (const std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
