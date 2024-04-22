// SPDX-License-Identifier: GPL-2.0-or-later

#include "check.hpp"

#include <common/error.hpp>
#include <common/types.hpp>
#include <core/generic/application.hpp>
#include <core/linux/device/hidraw.hpp>
#include <core/linux/runner.hpp>
#include <ipts/device.hpp>

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
	CLI::App app {"Utility for checking if a hidraw device is an IPTS touch device"};

	std::filesystem::path path {};
	app.add_option("DEVICE", path)
		->description("The hidraw device node to check")
		->type_name("FILE")
		->required();

	bool quiet = false;
	app.add_flag("-q,--quiet", quiet)->description("Disable output of device information");

	const std::set<std::string> allowed_types {"any", "touchscreen", "touchpad"};
	const std::map<std::string, std::optional<ipts::Device::Type>> type_map = {
		{"any", std::nullopt},
		{"touchscreen", ipts::Device::Type::Touchscreen},
		{"touchpad", ipts::Device::Type::Touchpad},
	};

	std::string type = "any";
	app.add_option("-t,--type", type)
		->description("Whether to look for a specific type of device")
		->transform(CLI::IsMember(allowed_types));

	app.get_formatter()->column_width(45);

	CLI11_PARSE(app, argc, argv);

	if (quiet)
		spdlog::set_level(spdlog::level::off);

	const std::optional<ipts::Device::Type> target_type = type_map.at(type);

	/*
	 * Create a dummy application that reads from the device.
	 * If the device is not an IPTS device, this will fail and throw an exception.
	 */
	const core::linux::Runner<Check, core::linux::device::Hidraw> check {path, target_type};

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
	} catch (const std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
