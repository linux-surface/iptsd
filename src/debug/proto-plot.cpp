#include "gfx/cairo.hpp"
#include "visualization.hpp"

#include <contacts/container/image.hpp>
#include <contacts/eval/perf.hpp>
#include <contacts/processor.hpp>
#include <contacts/types.hpp>
#include <ipts/parser.hpp>

#include <CLI/CLI.hpp>
#include <filesystem>
#include <fmt/core.h>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <span>
#include <spdlog/spdlog.h>
#include <vector>

using namespace iptsd::gfx;

enum class mode_type {
	plot,
	perf,
};

static int plot_main(int argc, char *argv[])
{
	auto mode = mode_type::plot;
	auto path_in = std::string {};
	auto path_out = std::string {};

	auto app = CLI::App {"Digitizer Prototype -- Plotter"};
	app.failure_message(CLI::FailureMessage::help);
	app.set_help_all_flag("--help-all", "Show full help message");
	app.require_subcommand(1);

	auto cmd_plot = app.add_subcommand("plot", "Plot results to PNG files");
	cmd_plot->callback([&]() { mode = mode_type::plot; });
	cmd_plot->add_option("input", path_in, "Input file")->required();
	cmd_plot->add_option("output", path_out, "Output directory")->required();

	auto cmd_perf = app.add_subcommand("perf", "Evaluate performance");
	cmd_perf->callback([&]() { mode = mode_type::perf; });
	cmd_perf->add_option("input", path_in, "Input file")->required();

	CLI11_PARSE(app, argc, argv);

	std::ifstream ifs;
	ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	ifs.open(path_in, std::ios::binary | std::ios::ate);

	std::streamsize size = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	IptsParser parser(size);

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	ifs.read(reinterpret_cast<char *>(parser.buffer().data()), size);

	std::vector<iptsd::container::Image<f32>> heatmaps;

	parser.on_heatmap = [&](const auto &data) {
		iptsd::index2_t size = {data.width, data.height};

		iptsd::container::Image<f32> hm {size};
		std::transform(data.data.begin(), data.data.end(), hm.begin(), [&](auto v) {
			f32 val = static_cast<f32>(v - data.z_min) /
				  static_cast<f32>(data.z_max - data.z_min);

			return 1.0f - val;
		});

		heatmaps.push_back(hm);
	};

	parser.parse_loop();

	if (heatmaps.empty()) {
		spdlog::warn("No touch data found!");
		return 0;
	}

	iptsd::TouchProcessor proc {heatmaps[0].size()};

	std::vector<iptsd::container::Image<f32>> out;
	out.reserve(heatmaps.size());

	std::vector<std::vector<iptsd::TouchPoint>> out_tp;
	out_tp.reserve(heatmaps.size());

	spdlog::info("Processing...");

	int i = 0;
	do {
		for (auto const &hm : heatmaps) {
			auto const &tp = proc.process(hm);

			out.push_back(hm);
			out_tp.push_back(tp);
		}
	} while (++i < 50 && mode == mode_type::perf);

	// statistics
	spdlog::info("Performance Statistics:");

	for (auto const &e : proc.perf().entries()) {
		using ms = std::chrono::microseconds;

		spdlog::info("  {}", e.name);
		spdlog::info("    N:      {:8d}", e.n_measurements);
		spdlog::info("    full:   {:8d}", e.total<ms>().count());
		spdlog::info("    mean:   {:8d}", e.mean<ms>().count());
		spdlog::info("    stddev: {:8d}", e.stddev<ms>().count());
		spdlog::info("    min:    {:8d}", e.min<ms>().count());
		spdlog::info("    max:    {:8d}", e.max<ms>().count());
		spdlog::info("");
	}

	if (mode == mode_type::perf)
		return 0;

	// plot
	spdlog::info("Plotting...");

	auto const width = 900;
	auto const height = 600;

	auto const dir_out = std::filesystem::path {path_out};
	std::filesystem::create_directories(dir_out);

	auto surface = cairo::image_surface_create(cairo::Format::Argb32, {width, height});
	auto cr = cairo::Cairo::create(surface);

	iptsd::Visualization vis {heatmaps[0].size()};

	for (std::size_t i = 0; i < out.size(); ++i) {
		vis.draw(cr, out[i], out_tp[i], width, height);

		// write file
		auto fname = std::array<char, 32> {};
		fmt::format_to_n(fname.begin(), fname.size(), "out-{:04d}.png", i);

		surface.write_to_png(dir_out / fname.data());
	}

	return 0;
}

int main(int argc, char *argv[])
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return plot_main(argc, argv);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
