// SPDX-License-Identifier: GPL-2.0-or-later

#include "dump.hpp"

#include <common/signal.hpp>
#include <common/types.hpp>
#include <core/linux/device-runner.hpp>

#include <CLI/CLI.hpp>
#include <cstdlib>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

namespace iptsd::apps::dump {

static int main(const gsl::span<char *> args)
{
	CLI::App app {};
	std::filesystem::path path {};
	std::filesystem::path output {};

	app.add_option("DEVICE", path, "The hidraw device to read from.")
		->type_name("FILE")
		->required();

	app.add_option("OUTPUT", output, "Output binary data to this file.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	// Create a dumping application that reads from a device.
	core::linux::DeviceRunner<Dump> dump {path, output};

	const auto _sigterm = common::signal<SIGTERM>([&](int) { dump.stop(); });
	const auto _sigint = common::signal<SIGINT>([&](int) { dump.stop(); });

	if (!dump.run())
		return EXIT_FAILURE;

	return 0;
}

} // namespace iptsd::apps::dump

int main(int argc, char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");
	const gsl::span<char *> args {argv, gsl::narrow<usize>(argc)};

	try {
		return iptsd::apps::dump::main(args);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
