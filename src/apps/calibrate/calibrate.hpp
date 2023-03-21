// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_CALIBRATE_CALIBRATE_HPP
#define IPTSD_APPS_CALIBRATE_CALIBRATE_HPP

#include <core/generic/application.hpp>
#include <core/generic/config.hpp>

#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>

namespace iptsd::apps::calibrate {

class Calibrate : public core::Application {
private:
	// The sizes of all received contacts.
	std::vector<f64> m_size {};

	// The aspect ratios of all received contacts.
	std::vector<f64> m_aspect {};

	f64 m_size_sum = 0;
	f64 m_aspect_sum = 0;

	// The diagonal of the screen (in centimeters).
	f64 m_diagonal;

public:
	Calibrate(const core::Config &config,
		  const core::DeviceInfo &info,
		  std::optional<const ipts::Metadata> metadata)
		: core::Application(config, info, metadata)
		, m_diagonal {std::hypot(config.width, config.height)} {};

	void on_start() override
	{
		spdlog::info("Samples: 0");
		spdlog::info("Size:    0.000 (Min: 0.000; Max: 0.000)");
		spdlog::info("Aspect:  0.000 (Min: 0.000; Max: 0.000)");
	}

	void on_contacts(const std::vector<contacts::Contact<f64>> &contacts) override
	{
		// Calculate size and aspect of all stable contacts
		for (const contacts::Contact<f64> &contact : contacts) {
			if (!contact.stable.value_or(true))
				continue;

			const auto size = contact.size.maxCoeff() * m_diagonal;
			const auto aspect = contact.size.maxCoeff() / contact.size.minCoeff();

			m_size_sum += size;
			m_aspect_sum += size;

			m_size.push_back(size);
			m_aspect.push_back(aspect);
		}

		if (m_size.empty())
			return;

		std::sort(m_size.begin(), m_size.end());
		std::sort(m_aspect.begin(), m_aspect.end());

		const f64 size_s = gsl::narrow<f64>(m_size.size());
		const f64 size_a = gsl::narrow<f64>(m_aspect.size());

		const f64 avg_s = m_size_sum / size_s;
		const f64 avg_a = m_aspect_sum / size_a;

		// Determine 1st and 99th percentile
		const f64 min_idx = std::max(size_s - 1, 0.0) * 0.01;
		const f64 max_idx = std::max(size_s - 1, 0.0) * 0.99;

		const f64 min_s = m_size[gsl::narrow<usize>(std::round(min_idx))];
		const f64 max_s = m_size[gsl::narrow<usize>(std::round(max_idx))];

		const f64 min_a = m_aspect[gsl::narrow<usize>(std::round(min_idx))];
		const f64 max_a = m_aspect[gsl::narrow<usize>(std::round(max_idx))];

		// Reset console output
		std::cout << "\033[A"; // Move cursor up one line
		std::cout << "\33[2K"; // Erase current line
		std::cout << "\033[A"; // Move cursor up one line
		std::cout << "\33[2K"; // Erase current line
		std::cout << "\033[A"; // Move cursor up one line
		std::cout << "\33[2K"; // Erase current line
		std::cout << "\r";     // Move cursor to the left

		spdlog::info("Samples: {}", m_size.size());
		spdlog::info("Size:    {:.3f} (Min: {:.3f}; Max: {:.3f})", avg_s, min_s, max_s);
		spdlog::info("Aspect:  {:.3f} (Min: {:.3f}; Max: {:.3f})", avg_a, min_a, max_a);
	}
};

} // namespace iptsd::apps::calibrate

#endif // IPTSD_APPS_CALIBRATE_CALIBRATE_HPP
