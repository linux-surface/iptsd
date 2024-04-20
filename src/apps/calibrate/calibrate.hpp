// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_CALIBRATE_CALIBRATE_HPP
#define IPTSD_APPS_CALIBRATE_CALIBRATE_HPP

#include <common/buildopts.hpp>
#include <common/casts.hpp>
#include <common/chrono.hpp>
#include <common/types.hpp>
#include <contacts/contact.hpp>
#include <core/generic/application.hpp>
#include <core/generic/config.hpp>
#include <core/generic/device.hpp>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace iptsd::apps::calibrate {

class Calibrate : public core::Application {
private:
	using clock = chrono::system_clock;

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
	Calibrate(const core::Config &config, const core::DeviceInfo &info)
		: core::Application(config, info),
		  m_diagonal {std::hypot(config.width, config.height)} {};

	void on_start() override
	{
		spdlog::info("Samples: 0");
		spdlog::info("Size:    0.000 (Min: 0.000; Max: 0.000)");
		spdlog::info("Aspect:  0.000 (Min: 0.000; Max: 0.000)");
	}

	void on_touch(const std::vector<contacts::Contact<f64>> &contacts) override
	{
		// Calculate size and aspect of all stable contacts
		for (const contacts::Contact<f64> &contact : contacts) {
			if (!contact.stable.value_or(true))
				continue;

			const auto size = contact.size.maxCoeff() * m_diagonal;
			const auto aspect = contact.size.maxCoeff() / contact.size.minCoeff();

			m_size_sum += size;
			m_aspect_sum += aspect;

			m_size.push_back(size);
			m_aspect.push_back(aspect);
		}

		if (m_size.empty())
			return;

		std::sort(m_size.begin(), m_size.end());
		std::sort(m_aspect.begin(), m_aspect.end());

		const f64 size = casts::to<f64>(m_size.size());

		const f64 avg_s = m_size_sum / size;
		const f64 avg_a = m_aspect_sum / size;

		f64 min_s {};
		f64 max_s {};

		f64 min_a {};
		f64 max_a {};

		this->calculate_min_max(min_s, max_s, min_a, max_a);

		// Reset console output
		std::cout << "\033[A"; // Move cursor up one line
		std::cout << "\33[2K"; // Erase current line
		std::cout << "\033[A"; // Move cursor up one line
		std::cout << "\33[2K"; // Erase current line
		std::cout << "\033[A"; // Move cursor up one line
		std::cout << "\33[2K"; // Erase current line
		std::cout << "\r";     // Move cursor to the left

		spdlog::info("Samples: {}", size);
		spdlog::info("Size:    {:.3f} (Min: {:.3f}; Max: {:.3f})", avg_s, min_s, max_s);
		spdlog::info("Aspect:  {:.3f} (Min: {:.3f}; Max: {:.3f})", avg_a, min_a, max_a);
	}

	void on_stop() override
	{
		const clock::duration now = clock::now().time_since_epoch();
		usize unix = chrono::duration_cast<seconds<usize>>(now).count();

		const u16 vendor = m_info.vendor;
		const u16 product = m_info.product;

		const std::string devtime = fmt::format("{:04X}_{:04X}_{}", vendor, product, unix);

		const std::string no_slack = fmt::format("iptsd_calib_{}_0mm.conf", devtime);
		const std::string some_slack = fmt::format("iptsd_calib_{}_2mm.conf", devtime);
		const std::string much_slack = fmt::format("iptsd_calib_{}_10mm.conf", devtime);

		this->write_file(no_slack, 0.0);
		this->write_file(some_slack, 0.1);
		this->write_file(much_slack, 0.5);

		// clang-format off

		const std::string filename =
			fmt::format("{}/91-calibration-{:04X}-{:04X}.conf", common::buildopts::ConfigDir, vendor, product);

		spdlog::info("");
		spdlog::info("To finish the calibration process, apply the determined values to iptsd.");
		spdlog::info("Three config snippets have been generated for you in the current directory.");
		spdlog::info("Run the displayed to command to install them, and restart iptsd.");
		spdlog::info("");
		spdlog::info("Recommended:");
		spdlog::info("    sudo cp {} {}", some_slack, filename);
		spdlog::info("");
		spdlog::info("If iptsd misses inputs:");
		spdlog::info("    sudo cp {} {}", much_slack, filename);
		spdlog::info("");
		spdlog::info("For manual finetuning:");
		spdlog::info("    sudo cp {} {}", no_slack, filename);
		spdlog::info("");
		spdlog::warn("Running these commands can permanently overwrite a previous calibration!");

		// clang-format on
	}

	void calculate_min_max(f64 &size_min, f64 &size_max, f64 &aspect_min, f64 &aspect_max) const
	{
		const f64 size = casts::to<f64>(m_size.size());

		// Determine 1st and 99th percentile
		const f64 min_idx = std::max(size - 1, 0.0) * 0.01;
		const f64 max_idx = std::max(size - 1, 0.0) * 0.99;

		size_min = m_size[casts::to<usize>(std::round(min_idx))];
		size_max = m_size[casts::to<usize>(std::round(max_idx))];

		aspect_min = m_aspect[casts::to<usize>(std::round(min_idx))];
		aspect_max = m_aspect[casts::to<usize>(std::round(max_idx))];
	}

	void write_file(const std::filesystem::path &out, const f64 slack) const
	{
		std::ofstream writer {out};

		const f64 size = casts::to<f64>(m_size.size());

		f64 size_min {};
		f64 size_max {};

		f64 aspect_min {};
		f64 aspect_max {};

		this->calculate_min_max(size_min, size_max, aspect_min, aspect_max);

		// Apply slack
		if (slack > 0) {
			size_min -= slack;
			size_max += slack;

			size_min = std::max(size_min, 0.0);
			aspect_min = std::max(aspect_min, 1.0);

			size_min = std::floor(size_min * 10) / 10;
			size_max = std::ceil(size_max * 10) / 10;

			aspect_min = std::floor(aspect_min * 10) / 10;
			aspect_max = std::ceil(aspect_max * 10) / 10;
		}

		writer << "#\n";
		writer << "# Samples: " << fmt::format("{}", size) << "\n";
		writer << "# Slack:   " << fmt::format("{:.3f}", slack) << "\n";
		writer << "#\n";
		writer << "\n";
		writer << "[Device]\n";
		writer << "Vendor = " << fmt::format("0x{:04X}", m_info.vendor) << "\n";
		writer << "Product = " << fmt::format("0x{:04X}", m_info.product) << "\n";
		writer << "\n";
		writer << "[Contacts]\n";
		writer << "SizeMin = " << fmt::format("{:.3f}", size_min) << "\n";
		writer << "SizeMax = " << fmt::format("{:.3f}", size_max) << "\n";
		writer << "AspectMin = " << fmt::format("{:.3f}", aspect_min) << "\n";
		writer << "AspectMax = " << fmt::format("{:.3f}", aspect_max) << "\n";

		writer.close();
	}
};

} // namespace iptsd::apps::calibrate

#endif // IPTSD_APPS_CALIBRATE_CALIBRATE_HPP
