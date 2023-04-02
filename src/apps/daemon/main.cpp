// SPDX-License-Identifier: GPL-2.0-or-later

#include "daemon.hpp"

#include <common/types.hpp>
#include <core/linux/device-runner.hpp>
#include <core/linux/signal-handler.hpp>

#include <CLI/CLI.hpp>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include <cstdlib>

namespace iptsd::apps::daemon {

static int run(const gsl::span<char *> args)
{
	CLI::App app {"Daemon to translate touchscreen inputs to Linux input events."};

	std::filesystem::path path {};
	app.add_option("DEVICE", path)
		->description("The hidraw device node of the touchscreen.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	// Create a daemon application that reads from a device.
	core::linux::DeviceRunner<Daemon> daemon {path};

	const auto _sigterm = core::linux::signal<SIGTERM>([&](int) { daemon.stop(); });
	const auto _sigint = core::linux::signal<SIGINT>([&](int) { daemon.stop(); });

	if (!daemon.run())
		return EXIT_FAILURE;

	return 0;
}

} // namespace iptsd::apps::daemon

int main(int argc, char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");
	const gsl::span<char *> args {argv, gsl::narrow<usize>(argc)};

	try {
		return iptsd::apps::daemon::run(args);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
