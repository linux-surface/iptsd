// SPDX-License-Identifier: GPL-2.0-or-later

#include "container/ops.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/finder.hpp>
#include <container/image.hpp>
#include <ipts/parser.hpp>

#include <CLI/CLI.hpp>
#include <algorithm>
#include <chrono>
#include <exception>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

namespace iptsd::debug::perf {

struct iptsd_dump_header {
	i16 vendor;
	i16 product;
	std::size_t buffer_size;
};

static void iptsd_perf_handle_input(contacts::ContactFinder &finder, const ipts::Heatmap &data)
{
	// Make sure that all buffers have the correct size
	finder.resize(index2_t {data.dim.width, data.dim.height});

	// Normalize and invert the heatmap data.
	std::transform(data.data.begin(), data.data.end(), finder.data().begin(), [&](auto v) {
		f32 val = static_cast<f32>(v - data.dim.z_min) /
			  static_cast<f32>(data.dim.z_max - data.dim.z_min);

		return 1.0f - val;
	});

	// Search for a contact
	finder.search();
}

static int main(gsl::span<char *> args)
{
	CLI::App app {};
	std::filesystem::path path;

	app.add_option("DATA", path, "The binary data file containing the data to test.")
		->type_name("FILE")
		->required();

	CLI11_PARSE(app, args.size(), args.data());

	std::ifstream ifs {};
	ifs.open(path, std::ios::in | std::ios::binary);

	struct iptsd_dump_header header {};

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	ifs.read(reinterpret_cast<char *>(&header), sizeof(header));

	spdlog::info("Vendor:       {:04X}", header.vendor);
	spdlog::info("Product:      {:04X}", header.product);
	spdlog::info("Buffer Size:  {}", header.buffer_size);

	config::Config config {header.vendor, header.product};

	// Check if a config was found
	if (config.width == 0 || config.height == 0)
		throw std::runtime_error("No display config for this device was found!");

	contacts::ContactFinder finder {config.contacts()};

	ipts::Parser parser {};
	parser.on_heatmap = [&](const auto &data) { iptsd_perf_handle_input(finder, data); };

	std::vector<u8> buffer(header.buffer_size);
	std::vector<f64> measurements {};

	while (ifs.peek() != EOF) {
		try {
			ssize_t size = 0;

			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			ifs.read(reinterpret_cast<char *>(&size), sizeof(size));

			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			ifs.read(reinterpret_cast<char *>(buffer.data()),
				 gsl::narrow<std::streamsize>(buffer.size()));

			// Take start time
			auto start = std::chrono::high_resolution_clock::now();

			parser.parse(gsl::span<u8>(buffer.data(), size));

			// Take end time
			auto end = std::chrono::high_resolution_clock::now();

			// Save measurement
			measurements.push_back(gsl::narrow<f64>((end - start).count()));
		} catch (std::exception &e) {
			spdlog::warn(e.what());
			continue;
		}
	}

	auto total = container::ops::sum(measurements);
	auto [min, max] = container::ops::minmax(measurements);

	f64 average = total / gsl::narrow<f64>(measurements.size());

	spdlog::info("Total:   {:.3f}μs", total / 1000);
	spdlog::info("Average: {:.3f}μs", average / 1000);
	spdlog::info("Minimum: {:.3f}μs", min / 1000);
	spdlog::info("Maximum: {:.3f}μs", max / 1000);

	return 0;
}

} // namespace iptsd::debug::perf

int main(int argc, char *argv[])
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::debug::perf::main(gsl::span(argv, argc));
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
