// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_VISUALIZATION_VISUALIZE_PNG_HPP
#define IPTSD_APPS_VISUALIZATION_VISUALIZE_PNG_HPP

#include "visualize.hpp"

#include <common/casts.hpp>
#include <core/generic/config.hpp>
#include <core/generic/device.hpp>

#include <cairomm/cairomm.h>
#include <gsl/gsl>

#include <cmath>
#include <filesystem>
#include <utility>

namespace iptsd::apps::visualization {

class VisualizePNG : public Visualize {
private:
	std::filesystem::path m_output;
	Cairo::RefPtr<Cairo::ImageSurface> m_tex {};

	usize m_counter = 0;

public:
	VisualizePNG(const core::Config &config,
	             const core::DeviceInfo &info,
	             std::filesystem::path output)
		: Visualize(config, info),
		  m_output {std::move(output)} {};

	void on_start() override
	{
		const f64 aspect = m_config.width / m_config.height;

		// Determine output resolution.
		const f64 y = 1000;
		const f64 x = y * aspect;

		m_size.x() = casts::to<i32>(std::round(x));
		m_size.y() = casts::to<i32>(std::round(y));

		// Create a texture for drawing.
		m_tex = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, m_size.x(), m_size.y());

		// Create context for issuing draw commands.
		m_cairo = Cairo::Context::create(m_tex);

		std::filesystem::create_directories(m_output);
	}

	void on_data(const gsl::span<u8> data) override
	{
		Visualize::on_data(data);

		this->draw();

		// Save the texture to a png file
		m_tex->write_to_png(m_output / fmt::format("{:05}.png", m_counter++));
	}
};

} // namespace iptsd::apps::visualization

#endif // IPTSD_APPS_VISUALIZATION_VISUALIZE_PNG_HPP
