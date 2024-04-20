// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_PERF_PERF_HPP
#define IPTSD_APPS_PERF_PERF_HPP

#include <common/chrono.hpp>
#include <common/types.hpp>
#include <contacts/finder.hpp>
#include <core/generic/application.hpp>
#include <core/generic/config.hpp>

#include <gsl/gsl>

#include <algorithm>
#include <utility>
#include <vector>

namespace iptsd::apps::perf {

class Perf : public core::Application {
private:
	using clock = chrono::steady_clock;

public:
	usize total = 0;
	usize total_of_squares = 0;
	usize count = 0;

	clock::duration min = clock::duration::max();
	clock::duration max = clock::duration::min();

private:
	bool m_had_touch {};

public:
	Perf(const core::Config &config, const core::DeviceInfo &info)
		: core::Application(config, info) {};

	void on_touch(const std::vector<contacts::Contact<f64>> & /* unused */) override
	{
		m_had_touch = true;
	}

	void on_data(const gsl::span<u8> data) override
	{
		// Take start time
		const clock::time_point start = clock::now();

		// Send the report to the finder through the parser for processing
		core::Application::on_data(data);

		if (std::exchange(m_had_touch, false)) {
			// Take end time
			const clock::time_point end = clock::now();
			const clock::duration x_ns = end - start;

			// Divide early for x and x**2 because they are overflowing
			const usize x_us = chrono::duration_cast<microseconds<usize>>(x_ns).count();

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

		total = 0;
		total_of_squares = 0;
		count = 0;

		min = clock::duration::max();
		max = clock::duration::min();
	}
};

} // namespace iptsd::apps::perf

#endif // IPTSD_APPS_PERF_PERF_HPP
