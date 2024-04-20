// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_VISUALIZATION_VISUALIZE_HPP
#define IPTSD_APPS_VISUALIZATION_VISUALIZE_HPP

#include <common/casts.hpp>
#include <common/types.hpp>
#include <contacts/contact.hpp>
#include <core/generic/application.hpp>
#include <core/generic/config.hpp>
#include <core/generic/device.hpp>
#include <ipts/samples/stylus.hpp>

#include <cairomm/cairomm.h>
#include <gsl/gsl>
#include <spdlog/spdlog.h>

#include <cmath>
#include <deque>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace iptsd::apps::visualization {

class Visualize : public core::Application {
private:
	Image<u32> m_argb {};

	// The last known state of the stylus.
	std::deque<ipts::samples::Stylus> m_history {};

protected:
	// The size of the texture we are drawing to.
	Vector2<i32> m_size {};

	// The cairo context for drawing.
	Cairo::RefPtr<Cairo::Context> m_cairo {};

public:
	Visualize(const core::Config &config, const core::DeviceInfo &info)
		: core::Application(config, info) {};

	void on_touch(const std::vector<contacts::Contact<f64>> & /* unused */) override
	{
		const Eigen::Index cols = m_heatmap.cols();
		const Eigen::Index rows = m_heatmap.rows();

		if (m_argb.rows() != rows || m_argb.cols() != cols)
			m_argb.conservativeResize(rows, cols);

		// Convert floating point values of range [0, 1] to greyscale ARGB.
		for (Eigen::Index y = 0; y < rows; y++) {
			for (Eigen::Index x = 0; x < cols; x++) {
				const f64 value = m_heatmap(y, x);

				constexpr u8 max = std::numeric_limits<u8>::max();
				const u8 v = casts::to<u8>(std::round(value * max));

				constexpr u32 a = max;
				const u32 r = v;
				const u32 g = v;
				const u32 b = v;

				m_argb(y, x) = (a << 24) + (r << 16) + (g << 8) + b;
			}
		}
	}

	void on_stylus(const ipts::samples::Stylus &data) override
	{
		if (!data.proximity) {
			m_history.clear();
			return;
		}

		m_history.push_back(data);

		if (m_history.size() < 50)
			return;

		m_history.pop_front();
	}

	void draw()
	{
		// Draw the raw heatmap
		this->draw_heatmap();

		// Draw the contacts
		this->draw_contacts();

		// Draw the position of the stylus
		this->draw_stylus();

		// Draw a line through the last 50 positions of the stylus
		this->draw_stylus_stroke();
	}

	void draw_heatmap()
	{
		if (m_argb.size() == 0) {
			m_cairo->set_source_rgb(0, 0, 0);
			m_cairo->paint();
			return;
		}

		const i32 cols = casts::to<i32>(m_argb.cols());
		const i32 rows = casts::to<i32>(m_argb.rows());

		constexpr auto format = Cairo::FORMAT_ARGB32;
		const auto stride = Cairo::ImageSurface::format_stride_for_width(format, cols);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		auto *data = reinterpret_cast<u8 *>(m_argb.data());

		// Create Cairo surface based on data buffer.
		const Cairo::RefPtr<Cairo::ImageSurface> source =
			Cairo::ImageSurface::create(data, format, cols, rows, stride);

		const f64 wx = casts::to<f64>(m_size.x());
		const f64 wy = casts::to<f64>(m_size.y());

		f64 sx = casts::to<f64>(cols) / wx;
		f64 sy = casts::to<f64>(rows) / wy;

		f64 tx = 0;
		f64 ty = 0;

		if (m_config.invert_x) {
			sx = -sx;
			tx = cols;
		}

		if (m_config.invert_y) {
			sy = -sy;
			ty = rows;
		}

		Cairo::Matrix matrix = Cairo::identity_matrix();
		matrix.translate(tx, ty);
		matrix.scale(sx, sy);

		// Upscale surface to window dimensions
		const auto pattern = Cairo::SurfacePattern::create(source);
		pattern->set_matrix(matrix);
		pattern->set_filter(Cairo::FILTER_NEAREST);

		// Copy source into output
		m_cairo->set_source(pattern);
		m_cairo->rectangle(0, 0, wx, wy);
		m_cairo->fill();
	}

	void draw_contacts() const
	{
		const f64 diag = m_size.cast<f64>().hypotNorm();

		// Select Font
		m_cairo->select_font_face("monospace",
		                          Cairo::FONT_SLANT_NORMAL,
		                          Cairo::FONT_WEIGHT_NORMAL);
		m_cairo->set_font_size(24.0);

		for (const auto &contact : m_contacts) {
			/*
			 * Red: Invalid
			 * Yellow: Unstable
			 * Green: OK
			 */
			if (!contact.valid.value_or(true))
				m_cairo->set_source_rgb(1, 0, 0);
			else if (!contact.stable.value_or(true))
				m_cairo->set_source_rgb(1, 1, 0);
			else
				m_cairo->set_source_rgb(0, 1, 0);

			const std::string index = fmt::format("{:02}", contact.index.value_or(0));

			Cairo::TextExtents extends {};
			m_cairo->get_text_extents(index, extends);

			const Vector2<f64> mean = contact.mean.cwiseProduct(m_size.cast<f64>());
			const f64 orientation = contact.orientation * M_PI;

			const Vector2<f64> size = (contact.size.array() * diag).matrix();

			// Center the text at the mean point of the contact
			m_cairo->move_to(mean.x() - (extends.x_bearing + extends.width / 2),
			                 mean.y() - (extends.y_bearing + extends.height / 2));
			m_cairo->save();

			m_cairo->show_text(index);
			m_cairo->restore();
			m_cairo->stroke();

			m_cairo->move_to(mean.x(), mean.y());
			m_cairo->save();

			m_cairo->translate(mean.x(), mean.y());
			m_cairo->rotate(-orientation);
			m_cairo->scale(size.maxCoeff(), size.minCoeff());
			m_cairo->begin_new_sub_path();
			m_cairo->arc(0, 0, 1, 0, 2 * M_PI);

			m_cairo->restore();
			m_cairo->stroke();
		}
	}

	void draw_stylus()
	{
		if (m_history.empty())
			return;

		const ipts::samples::Stylus &stylus = m_history.back();

		if (!stylus.proximity)
			return;

		constexpr f64 RADIUS = 50;

		const f64 sx = stylus.x * (m_size.x() - 1);
		const f64 sy = stylus.y * (m_size.y() - 1);

		m_cairo->set_source_rgb(0, 1, 0.5);

		// Draw a cross for pen, a box for rubber, and a triangle for the side button
		if (!stylus.rubber && !stylus.button) {
			m_cairo->move_to(sx - RADIUS, sy);
			m_cairo->line_to(sx + RADIUS, sy);
			m_cairo->stroke();

			m_cairo->move_to(sx, sy - RADIUS);
			m_cairo->line_to(sx, sy + RADIUS);
			m_cairo->stroke();
		} else if (stylus.rubber && !stylus.button) {
			m_cairo->move_to(sx - RADIUS, sy - RADIUS);
			m_cairo->line_to(sx + RADIUS, sy - RADIUS);
			m_cairo->stroke();

			m_cairo->move_to(sx - RADIUS, sy + RADIUS);
			m_cairo->line_to(sx + RADIUS, sy + RADIUS);
			m_cairo->stroke();

			m_cairo->move_to(sx - RADIUS, sy - RADIUS);
			m_cairo->line_to(sx - RADIUS, sy + RADIUS);
			m_cairo->stroke();

			m_cairo->move_to(sx + RADIUS, sy - RADIUS);
			m_cairo->line_to(sx + RADIUS, sy + RADIUS);
			m_cairo->stroke();
		} else if (!stylus.rubber && stylus.button) {
			m_cairo->move_to(sx - RADIUS, sy - RADIUS);
			m_cairo->line_to(sx + RADIUS, sy - RADIUS);
			m_cairo->stroke();

			m_cairo->move_to(sx - RADIUS, sy - RADIUS);
			m_cairo->line_to(sx, sy + RADIUS);
			m_cairo->stroke();

			m_cairo->move_to(sx + RADIUS, sy - RADIUS);
			m_cairo->line_to(sx, sy + RADIUS);
			m_cairo->stroke();
		}

		if (!stylus.contact)
			return;

		m_cairo->set_source_rgb(1, 0.5, 0);

		m_cairo->arc(sx, sy, RADIUS * stylus.pressure, 0, 2 * M_PI);
		m_cairo->stroke();

		const f64 ox = RADIUS * std::cos(stylus.azimuth) * std::sin(stylus.altitude);
		const f64 oy = -RADIUS * std::sin(stylus.azimuth) * std::sin(stylus.altitude);

		m_cairo->move_to(sx, sy);
		m_cairo->line_to(sx + ox, sy + oy);
		m_cairo->stroke();
	}

	void draw_stylus_stroke()
	{
		if (m_history.empty())
			return;

		for (usize i = 0; i < m_history.size() - 1; i++) {
			const ipts::samples::Stylus &from = m_history[i];
			const ipts::samples::Stylus &to = m_history[i + 1];

			if (!from.proximity || !to.proximity)
				continue;

			if (!from.contact || !to.contact)
				m_cairo->set_source_rgba(0.5, 0, 1.0, 0.5);
			else
				m_cairo->set_source_rgba(0.5, 0, 1.0, 1.0);

			const f64 fx = from.x * (m_size.x() - 1);
			const f64 fy = from.y * (m_size.y() - 1);

			m_cairo->move_to(fx, fy);

			const f64 tx = to.x * (m_size.x() - 1);
			const f64 ty = to.y * (m_size.y() - 1);

			m_cairo->line_to(tx, ty);
			m_cairo->stroke();
		}
	}
};

} // namespace iptsd::apps::visualization

#endif // IPTSD_APPS_VISUALIZATION_VISUALIZE_HPP
