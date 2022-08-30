// SPDX-License-Identifier: GPL-2.0-or-later

#include <common/types.hpp>
#include <contacts/finder.hpp>
#include <container/image.hpp>
#include <gfx/visualization.hpp>
#include <ipts/parser.hpp>

#include <CLI/CLI.hpp>
#include <algorithm>
#include <cairomm/cairomm.h>
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

namespace iptsd::debug::plot {

struct iptsd_dump_header {
	i16 vendor;
	i16 product;
	std::size_t buffer_size;
};

static void iptsd_plot_handle_input(const Cairo::RefPtr<Cairo::Context> &cairo, index2_t rsize,
				    gfx::Visualization &vis, contacts::ContactFinder &finder,
				    const ipts::Heatmap &data)
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
	const std::vector<contacts::Contact> &contacts = finder.search();

	// Draw the raw heatmap
	vis.draw_heatmap(cairo, rsize, finder.data());

	// Draw the contacts
	vis.draw_contacts(cairo, rsize, contacts);
}

static int main(gsl::span<char *> args)
{
	CLI::App app {};
	std::filesystem::path path;
	std::filesystem::path output;

	app.add_option("DATA", path, "The binary data file containing the data to plot.")
		->type_name("FILE")
		->required();

	app.add_option("OUTPUT", output, "The output directory containg plotted frames.")
		->type_name("DIR")
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

	gfx::Visualization vis {config};
	contacts::ContactFinder finder {config.contacts()};

	index2_t rsize {};
	f64 aspect = config.width / config.height;

	// Determine the output resolution
	rsize.y = 1000;
	rsize.x = gsl::narrow<int>(std::round(aspect * rsize.y));

	// Create a texture for drawing
	auto drawtex = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, rsize.x, rsize.y);
	auto cairo = Cairo::Context::create(drawtex);

	ipts::Parser parser {};
	parser.on_heatmap = [&](const auto &data) {
		iptsd_plot_handle_input(cairo, rsize, vis, finder, data);
	};

	std::vector<u8> buffer(header.buffer_size);
	std::filesystem::create_directories(output);

	u32 i = 0;
	while (ifs.peek() != EOF) {
		try {
			ssize_t size = 0;

			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			ifs.read(reinterpret_cast<char *>(&size), sizeof(size));

			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
			ifs.read(reinterpret_cast<char *>(buffer.data()),
				 gsl::narrow<std::streamsize>(buffer.size()));

			parser.parse(gsl::span<u8>(buffer.data(), size));

			// Save the texture to a png file
			drawtex->write_to_png(output / fmt::format("{:05}.png", i++));
		} catch (std::exception &e) {
			spdlog::warn(e.what());
			continue;
		}
	}

	return 0;
}

} // namespace iptsd::debug::plot

int main(int argc, char *argv[])
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::debug::plot::main(gsl::span(argv, argc));
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
