// SPDX-License-Identifier: GPL-2.0-or-later

#include <common/types.hpp>
#include <core/generic/config.hpp>
#include <core/generic/device.hpp>
#include <core/linux/config-loader.hpp>
#include <core/linux/hidraw-device.hpp>
#include <hid/report.hpp>
#include <ipts/data.hpp>
#include <ipts/descriptor.hpp>
#include <ipts/device.hpp>

#include <CLI/CLI.hpp>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <memory>
#include <optional>
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

	// Open the device
	const auto device = std::make_shared<core::linux::HidrawDevice>(path);

	core::DeviceInfo info {};
	info.vendor = device->vendor();
	info.product = device->product();

	// Create IPTS interface
	const ipts::Device ipts {device};
	const ipts::Descriptor &descriptor = ipts.descriptor();
	const std::optional<const ipts::Metadata> metadata = ipts.metadata();

	info.buffer_size = ipts.buffer_size();

	spdlog::info("Opened device {:04X}:{:04X}", device->vendor(), device->product());

	// Check if the device can switch modes
	if (!descriptor.find_modesetting_report().has_value()) {
		spdlog::error("{} is not an IPTS device!", path.string());
		return EXIT_FAILURE;
	}

	// Check if the device can send touch data.
	if (descriptor.find_touch_data_reports().empty()) {
		spdlog::error("{} is not an IPTS device!", path.string());
		return EXIT_FAILURE;
	}

	// Check if the device has a config
	const core::linux::ConfigLoader loader {info, metadata};
	const core::Config config = loader.config();

	if (config.width == 0 || config.height == 0) {
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
