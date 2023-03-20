// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_VISUALIZATION_VISUALIZE_HPP
#define IPTSD_APPS_VISUALIZATION_VISUALIZE_HPP

#include <common/types.hpp>
#include <core/generic/application.hpp>
#include <core/generic/config.hpp>
#include <ipts/data.hpp>

#include <cairomm/cairomm.h>
#include <cmath>
#include <gsl/gsl>
#include <limits>
#include <utility>
#include <vector>

namespace iptsd::apps::visualization {

class Visualize : public core::Application {
private:
	Image<u32> m_argb {};

protected:
	// The size of the texture we are drawing to.
	Vector2<i32> m_size {};

	// The cairo context for drawing.
	Cairo::RefPtr<Cairo::Context> m_cairo {};

public:
	Visualize(const core::Config &config, const core::DeviceInfo &info,
		  std::optional<const ipts::Metadata> metadata)
		: core::Application(config, info, metadata) {};

	void on_contacts(const std::vector<contacts::Contact<f64>> &) override
	{
		const Eigen::Index cols = m_heatmap.cols();
		const Eigen::Index rows = m_heatmap.rows();

		if (m_argb.rows() != rows || m_argb.cols() != cols)
			m_argb.conservativeResize(rows, cols);

		// Convert floating point values of range [0, 1] to greyscale ARGB.
		for (Eigen::Index y = 0; y < rows; y++) {
			for (Eigen::Index x = 0; x < cols; x++) {
				const f64 value = m_heatmap(y, x);

				const u8 max = std::numeric_limits<u8>::max();
				const u8 v = gsl::narrow<u8>(std::round(value * max));

				const u32 a = max;
				const u32 r = v;
				const u32 g = v;
				const u32 b = v;

				m_argb(y, x) = (a << 24) + (r << 16) + (g << 8) + b;
			}
		}
	}

	void draw()
	{
		if (m_argb.size() == 0)
			return;

		// Draw the raw heatmap
		this->draw_heatmap();

		// Draw the contacts
		this->draw_contacts();
	}

	void draw_heatmap()
	{
		const i32 cols = gsl::narrow<i32>(m_argb.cols());
		const i32 rows = gsl::narrow<i32>(m_argb.rows());

		const auto format = Cairo::FORMAT_ARGB32;
		const auto stride = Cairo::ImageSurface::format_stride_for_width(format, cols);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		const auto data = reinterpret_cast<u8 *>(m_argb.data());

		// Create Cairo surface based on data buffer.
		const Cairo::RefPtr<Cairo::ImageSurface> source =
			Cairo::ImageSurface::create(data, format, cols, rows, stride);

		const f64 wx = static_cast<f64>(m_size.x());
		const f64 wy = static_cast<f64>(m_size.y());

		f64 sx = static_cast<f64>(cols) / wx;
		f64 sy = static_cast<f64>(rows) / wy;

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

	void draw_contacts()
	{
		const f64 diag = m_size.cast<f64>().hypotNorm();

		// Select Font
		m_cairo->select_font_face("monospace", Cairo::FONT_SLANT_NORMAL,
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

			Vector2<f64> mean = contact.mean;
			f64 orientation = contact.orientation;

			const Vector2<f64> size = (contact.size.array() * diag).matrix();

			if (m_config.invert_x)
				mean.x() = 1.0 - mean.x();

			if (m_config.invert_y)
				mean.y() = 1.0 - mean.y();

			if (m_config.invert_x != m_config.invert_y)
				orientation = 1.0 - orientation;

			mean = mean.cwiseProduct(m_size.cast<f64>());
			orientation = orientation * M_PI;

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
};

} // namespace iptsd::apps::visualization

#endif // IPTSD_APPS_VISUALIZATION_VISUALIZE_HPP
