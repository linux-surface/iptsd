// SPDX-License-Identifier: GPL-2.0-or-later

#include "daemon.hpp"

#include <common/signal.hpp>
#include <common/types.hpp>
#include <core/linux/device-runner.hpp>

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

namespace iptsd::apps::daemon {

static int main(const gsl::span<char *> args)
{
	CLI::App app {};
	std::filesystem::path path {};

	app.add_option("DEVICE", path, "The hidraw device to read from.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	// Create a daemon application that reads from a device.
	core::linux::DeviceRunner<Daemon> daemon {path};

	const auto _sigterm = common::signal<SIGTERM>([&](int) { daemon.stop(); });
	const auto _sigint = common::signal<SIGINT>([&](int) { daemon.stop(); });

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
		return iptsd::apps::daemon::main(args);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
