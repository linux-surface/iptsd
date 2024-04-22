// SPDX-License-Identifier: GPL-2.0-or-later

#include "perf.hpp"

#include <common/casts.hpp>
#include <common/chrono.hpp>
#include <common/types.hpp>
#include <core/linux/device/file.hpp>
#include <core/linux/runner.hpp>
#include <core/linux/signal-handler.hpp>

#include <CLI/CLI.hpp>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>

namespace iptsd::apps::perf {
namespace {

int run(const int argc, const char **argv)
{
	CLI::App app {"Utility for performance testing of iptsd"};

	std::filesystem::path path {};
	app.add_option("DATA", path)
		->description("A binary data file containing touch reports")
		->type_name("FILE")
		->required();

	usize runs {};
	app.add_option("RUNS", runs)
		->description("How many times data will be processed")
		->check(CLI::PositiveNumber)
		->default_val(10);

	CLI11_PARSE(app, argc, argv);

	// Create a performance testing application that reads from a file.
	core::linux::Runner<Perf, core::linux::device::File> perf {path};

	const auto _sigterm = core::linux::signal<SIGTERM>([&](int) { perf.stop(); });
	const auto _sigint = core::linux::signal<SIGINT>([&](int) { perf.stop(); });

	using clock = std::chrono::steady_clock;

	usize total = 0;
	usize total_of_squares = 0;
	usize count = 0;

	clock::duration min = clock::duration::max();
	clock::duration max = clock::duration::min();

	bool should_stop = false;

	for (usize i = 0; i < runs; i++) {
		should_stop = perf.run();

		Perf &papp = perf.application();

		total += papp.total;
		total_of_squares += papp.total_of_squares;
		count += papp.count;

		min = std::min(min, papp.min);
		max = std::max(max, papp.max);

		if (should_stop)
			break;

		papp.reset();
	}

	const f64 n = casts::to<f64>(count);
	const f64 mean = casts::to<f64>(total) / n;
	const f64 stddev = std::sqrt(casts::to<f64>(total_of_squares) / n - mean * mean);

	spdlog::info("Ran {} times", count);
	spdlog::info("Total: {}μs", total);
	spdlog::info("Mean: {:.2f}μs", mean);
	spdlog::info("Standard Deviation: {:.2f}μs", stddev);
	spdlog::info("Minimum: {:.3f}μs", chrono::duration_cast<microseconds<f64>>(min).count());
	spdlog::info("Maximum: {:.3f}μs", chrono::duration_cast<microseconds<f64>>(max).count());

	if (!should_stop)
		return EXIT_FAILURE;

	return 0;
}

} // namespace
} // namespace iptsd::apps::perf

int main(const int argc, const char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::apps::perf::run(argc, argv);
	} catch (const std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
