#include <common/types.hpp>
#include <contacts/eval/perf.hpp>
#include <contacts/processor.hpp>
#include <container/image.hpp>
#include <gfx/visualization.hpp>
#include <ipts/parser.hpp>

#include <CLI/CLI.hpp>
#include <cairomm/context.h>
#include <cairomm/enums.h>
#include <cairomm/surface.h>
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

namespace iptsd::debug::plot {

enum class mode_type {
	plot,
	perf,
};

static int main(int argc, char *argv[])
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

	ipts::Parser parser(size);

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	ifs.read(reinterpret_cast<char *>(parser.buffer().data()), size);

	std::vector<container::Image<f32>> heatmaps;

	parser.on_heatmap = [&](const auto &data) {
		index2_t size = {data.width, data.height};

		container::Image<f32> hm {size};
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

	contacts::TouchProcessor proc {heatmaps[0].size()};

	std::vector<container::Image<f32>> out;
	out.reserve(heatmaps.size());

	std::vector<std::vector<contacts::TouchPoint>> out_tp;
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

	auto surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, width, height);
	auto cr = Cairo::Context::create(surface);

	gfx::Visualization vis {heatmaps[0].size()};

	for (std::size_t i = 0; i < out.size(); ++i) {
		vis.draw(cr, out[i], out_tp[i], width, height);

		// write file
		surface->write_to_png(dir_out / fmt::format("out-{:04d}.png", i));
	}

	return 0;
}

} // namespace iptsd::debug::plot

int main(int argc, char *argv[])
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::debug::plot::main(argc, argv);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
