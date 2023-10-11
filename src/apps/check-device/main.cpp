// SPDX-License-Identifier: GPL-2.0-or-later

#include <common/types.hpp>
#include <core/generic/application.hpp>
#include <core/linux/device-runner.hpp>

#include <CLI/CLI.hpp>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>

namespace iptsd::apps::check {
namespace {

int run(const int argc, const char **argv)
{
	CLI::App app {"Utility for checking if a hidraw device is an IPTS touchscreen."};

	std::filesystem::path path {};
	app.add_option("DEVICE", path)
		->description("The hidraw device node of the device.")
		->type_name("FILE")
		->required();

	bool quiet = false;
	app.add_flag("-q,--quiet", quiet)->description("Disable output of device information.");

	CLI11_PARSE(app, argc, argv);

	if (quiet)
		spdlog::set_level(spdlog::level::off);

	try {
		// Create a dummy application that reads from the device.
		const core::linux::DeviceRunner<core::Application> dummy {path};
	} catch (std::exception & /* unused */) {
		spdlog::error("{} is not an IPTS device!", path.string());
		return EXIT_FAILURE;
	}

	spdlog::info("{} is an IPTS device!", path.string());
	return 0;
}

} // namespace
} // namespace iptsd::apps::check

int main(const int argc, const char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::apps::check::run(argc, argv);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
