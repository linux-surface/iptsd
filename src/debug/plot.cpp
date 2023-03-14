// SPDX-License-Identifier: GPL-2.0-or-later

#include "contacts/contact.hpp"

#include <common/types.hpp>
#include <config/loader.hpp>
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
			     meta->unknown_byte, u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7],
			     u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]);
	}

	config::Loader loader {header.vendor, header.product, meta};
	const config::Config config = loader.config();

	// Check if a config was found
	if (config.width == 0 || config.height == 0)
		throw std::runtime_error("No display config for this device was found!");

	gfx::Visualization vis {config};

	Image<f32> heatmap {};
	std::vector<contacts::Contact<f32>> contacts {};

	contacts::Finder<f32, f64> finder {config.contacts<f32>()};

	index2_t rsize {};
	const f64 aspect = config.width / config.height;

	// Determine the output resolution
	rsize.y = 1000;
	rsize.x = gsl::narrow<int>(std::round(aspect * rsize.y));

	// Create a texture for drawing
	const Cairo::RefPtr<Cairo::ImageSurface> drawtex =
		Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, rsize.x, rsize.y);

	// Create context for issuing draw commands
	const Cairo::RefPtr<Cairo::Context> cairo = Cairo::Context::create(drawtex);

	ipts::Parser parser {};
	parser.on_heatmap = [&](const ipts::Heatmap &data) {
		const Eigen::Index rows = index_cast(data.dim.height);
		const Eigen::Index cols = index_cast(data.dim.width);

		// Make sure the heatmap buffer has the right size
		if (heatmap.rows() != rows || heatmap.cols() != cols)
			heatmap.conservativeResize(data.dim.height, data.dim.width);

		// Map the buffer to an Eigen container
		Eigen::Map<const Image<u8>> mapped {data.data.data(), rows, cols};

		const f32 range = static_cast<f32>(data.dim.z_max - data.dim.z_min);

		// Normalize and invert the heatmap.
		heatmap = 1.0f - (mapped.cast<f32>() - static_cast<f32>(data.dim.z_min)) / range;

		// Search for contacts
		finder.find(heatmap, contacts);

		container::Image<f32> img {
			{gsl::narrow<index_t>(cols), gsl::narrow<index_t>(rows)}};
		for (Eigen::Index y = 0; y < rows; y++) {
			for (Eigen::Index x = 0; x < cols; x++) {
				img[{gsl::narrow<index_t>(x), gsl::narrow<index_t>(y)}] =
					heatmap(y, x);
			}
		}

		// Draw the raw heatmap
		vis.draw_heatmap(cairo, rsize, img);

		// Draw the contacts
		vis.draw_contacts(cairo, rsize, contacts);
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
