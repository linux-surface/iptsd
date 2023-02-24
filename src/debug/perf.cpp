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
#include <optional>
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
	u32 runs = 10;

	app.add_option("DATA", path, "The binary data file containing the data to test.")
		->type_name("FILE")
		->required();
	app.add_option("RUNS", runs, "Repeat this number of runs through the data.")
		->check(CLI::Range(1, 1000));

	CLI11_PARSE(app, args.size(), args.data());

	std::ifstream ifs {};
	ifs.open(path, std::ios::in | std::ios::binary);

	struct iptsd_dump_header header {};

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	ifs.read(reinterpret_cast<char *>(&header), sizeof(header));

	char has_meta = 0;
	ifs.read(&has_meta, sizeof(has_meta));

	// Read metadata
	std::optional<ipts::Metadata> meta = std::nullopt;
	if (has_meta) {
		ipts::Metadata m {};

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		ifs.read(reinterpret_cast<char *>(&m), sizeof(m));

		meta = m;
	}

	spdlog::info("Vendor:       {:04X}", header.vendor);
	spdlog::info("Product:      {:04X}", header.product);
	spdlog::info("Buffer Size:  {}", header.buffer_size);

	if (meta.has_value()) {
		const auto &m = meta;
		auto &t = m->transform;
		auto &u = m->unknown.unknown;

		spdlog::info("Metadata:");
		spdlog::info("rows={}, columns={}", m->size.rows, m->size.columns);
		spdlog::info("width={}, height={}", m->size.width, m->size.height);
		spdlog::info("transform=[{},{},{},{},{},{}]", t.xx, t.yx, t.tx, t.xy, t.yy, t.ty);
		spdlog::info("unknown={}, [{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}]",
			     m->unknown_byte, u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7], u[8],
			     u[9], u[10], u[11], u[12], u[13], u[14], u[15]);
	}

	config::Config config {header.vendor, header.product, meta};

	// Check if a config was found
	if (config.width == 0 || config.height == 0)
		throw std::runtime_error("No display config for this device was found!");

	// Read the file into memory to eliminate filesystem access as a variable
	std::noskipws(ifs);
	std::vector<u8> buffer {std::istream_iterator<u8>(ifs), std::istream_iterator<u8>()};
	ifs = {};

	using clock = std::chrono::high_resolution_clock;
	using std::chrono::duration_cast;
	using micros_u64 = std::chrono::duration<u64, std::micro>;
	using micros_f64 = std::chrono::duration<f64, std::micro>;

	u64 total = 0;
	u64 total_of_squares = 0;
	u32 count = 0;

	auto min = clock::duration::max();
	auto max = clock::duration::min();
	bool had_heatmap = false;
	bool reader_finished_successfully = false;

	// Parser is idempotent but ContactFinder is not
	contacts::ContactFinder finder {config.contacts()};
	ipts::Parser parser {};
	parser.on_heatmap = [&](const auto &data) {
		iptsd_perf_handle_input(finder, data);
		// Don't track time for non-heatmap
		had_heatmap = true;
	};

	for (u32 i = 0; i < runs; i++) {
		finder.reset();
		reader_finished_successfully = false;
		gsl::span<u8> reader(buffer.data(), buffer.size());
		while (true) {
			try {
				if (reader.empty()) {
					reader_finished_successfully = true;
					break;
				}

				size_t size = 0;
				if (reader.size() < sizeof(size))
					break;
				std::memcpy(&size, reader.data(), sizeof(size));
				reader = reader.subspan(sizeof(size));

				if (reader.size() < size)
					break;
				if (size > header.buffer_size)
					break;
				gsl::span<u8> data = reader.subspan(0, size);
				reader = reader.subspan(header.buffer_size);

				// Take start time
				auto start = clock::now();

				// Send the report to the finder through the parser for processing
				// Cannot put this in a loop because it is not a pure function
				// as it has things like temporal averaging
				parser.parse(data);

				if (std::exchange(had_heatmap, false)) {
					// Take end time
					auto end = clock::now();

					clock::duration x_ns = end - start;
					// Divide early for x and x**2 because they are overflowing
					u64 x_us = duration_cast<micros_u64>(x_ns).count();
					total += x_us;
					total_of_squares += x_us * x_us;
					min = std::min(min, x_ns);
					max = std::max(max, x_ns);
					++count;
				}
			} catch (std::exception &e) {
				spdlog::warn(e.what());
				continue;
			}
		}
	}
	// This is outside the loop to not spam the user
	// as it will be set to the same value every iteration of the loop
	if (!reader_finished_successfully)
		spdlog::warn("Leftover data at end of input");

	f64 n = gsl::narrow<f64>(count);
	f64 mean = gsl::narrow<f64>(total) / n;
	f64 stddev = std::sqrt(gsl::narrow<f64>(total_of_squares) / n - mean * mean);

	spdlog::info("Ran {} times", count);
	spdlog::info("Total: {}μs", total);
	spdlog::info("Mean: {:.2f}μs", mean);
	spdlog::info("Standard Deviation: {:.2f}μs", stddev);
	spdlog::info("Minimum: {:.3f}μs", duration_cast<micros_f64>(min).count());
	spdlog::info("Maximum: {:.3f}μs", duration_cast<micros_f64>(max).count());

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
