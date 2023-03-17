// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_PERF_PERF_HPP
#define IPTSD_APPS_PERF_PERF_HPP

#include <core/generic/application.hpp>
#include <core/generic/config.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <optional>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace iptsd::apps::perf {

class Perf : public core::Application {
private:
	using clock = std::chrono::high_resolution_clock;
	using usecs = std::chrono::duration<usize, std::micro>;

public:
	usize total = 0;
	usize total_of_squares = 0;
	usize count = 0;

	clock::duration min = clock::duration::max();
	clock::duration max = clock::duration::min();

private:
	bool m_had_heatmap {};

public:
	Perf(const core::Config &config, const core::DeviceInfo &info,
	     std::optional<const ipts::Metadata> metadata)
		: core::Application(config, info, metadata) {};

	void on_contacts(const std::vector<contacts::Contact<f64>> &) override
	{
		m_had_heatmap = true;
	}

	void on_data(const gsl::span<u8> &data) override
	{
		using std::chrono::duration_cast;

		// Take start time
		const clock::time_point start = clock::now();

		// Send the report to the finder through the parser for processing
		core::Application::on_data(data);

		if (std::exchange(m_had_heatmap, false)) {
			// Take end time
			const clock::time_point end = clock::now();
			const clock::duration x_ns = end - start;

			// Divide early for x and x**2 because they are overflowing
			const usize x_us = duration_cast<usecs>(x_ns).count();

			total += x_us;
			total_of_squares += x_us * x_us;

			min = std::min(min, x_ns);
			max = std::max(max, x_ns);

			++count;
		}
	}

	/*!
	 * Resets the contact finder.
	 *
	 * This has to be done after every iteration to prevent
	 * skewing the results due to finger tracking being different.
	 */
	void reset()
	{
		m_finder.reset();
	}
};

} // namespace iptsd::apps::perf

#endif // IPTSD_APPS_PERF_PERF_HPP
