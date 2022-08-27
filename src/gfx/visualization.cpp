// SPDX-License-Identifier: GPL-2.0-or-later

#include "visualization.hpp"

#include "cmap.hpp"
#include "color.hpp"

#include <common/types.hpp>
#include <contacts/advanced/detector.hpp>
#include <contacts/finder.hpp>
#include <container/image.hpp>

#include <cairo.h>
#include <cairomm/cairomm.h>
#include <cmath>
#include <fmt/format.h>
#include <memory>
#include <vector>

namespace iptsd::gfx {

Cairo::RefPtr<Cairo::ImageSurface> image_surface_create(container::Image<Argb> &image)
{
	auto const format = Cairo::FORMAT_ARGB32;
	auto const size = image.size();

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	auto const data = reinterpret_cast<u8 *>(image.data());

	auto const stride = Cairo::ImageSurface::format_stride_for_width(format, size.x);
	return Cairo::ImageSurface::create(data, format, size.x, size.y, stride);
}

void Visualization::draw_heatmap(const Cairo::RefPtr<Cairo::Context> &cairo, index2_t window,
				 const container::Image<f32> &heatmap)
{
	if (!this->data || this->data->size() != heatmap.size())
		this->data = std::make_unique<container::Image<gfx::Argb>>(heatmap.size());

	// Plot heatmap into data buffer
	cmap::grayscale.map_into(*this->data, heatmap, {{0, 1}});

	// Create Cairo surface based on data buffer
	Cairo::RefPtr<Cairo::ImageSurface> source = image_surface_create(*this->data);

	f64 sx = heatmap.size().x / static_cast<f64>(window.x);
	f64 sy = heatmap.size().y / static_cast<f64>(window.y);

	f64 tx = 0;
	f64 ty = 0;

	if (this->config.invert_x) {
		sx = -sx;
		tx = heatmap.size().x;
	}

	if (this->config.invert_y) {
		sy = -sy;
		ty = heatmap.size().y;
	}

	Cairo::Matrix matrix = Cairo::identity_matrix();
	matrix.translate(tx, ty);
	matrix.scale(sx, sy);

	// Upscale surface to window dimensions
	Cairo::RefPtr<Cairo::SurfacePattern> pattern = Cairo::SurfacePattern::create(source);
	pattern->set_matrix(matrix);
	pattern->set_filter(Cairo::FILTER_NEAREST);

	// Copy source into output
	cairo->set_source(pattern);
	cairo->rectangle(0, 0, window.x, window.y);
	cairo->fill();
}

void Visualization::draw_contacts(const Cairo::RefPtr<Cairo::Context> &cairo, index2_t window,
				  const std::vector<contacts::Contact> &contacts)
{
	f64 diag = std::hypot(window.x, window.y);

	// Select Font
	cairo->select_font_face("monospace", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
	cairo->set_font_size(24.0);

	for (const auto &contact : contacts) {
		if (!contact.active)
			continue;

		// Color palms red, instable contacts yellow, and stable contacts green
		if (contact.palm)
			cairo->set_source_rgb(1, 0, 0);
		else if (!contact.stable)
			cairo->set_source_rgb(1, 1, 0);
		else
			cairo->set_source_rgb(0, 1, 0);

		std::string index = fmt::format("{:02}", contact.index);

		Cairo::TextExtents extends {};
		cairo->get_text_extents(index, extends);

		f64 x = contact.x * window.x;
		f64 y = contact.y * window.y;

		// Center the text at the mean point of the contact
		cairo->move_to(x - (extends.x_bearing + extends.width / 2),
			       y - (extends.y_bearing + extends.height / 2));
		cairo->save();

		cairo->show_text(index);
		cairo->restore();
		cairo->stroke();

		cairo->move_to(x, y);
		cairo->save();

		cairo->translate(x, y);
		cairo->rotate(-contact.angle);
		cairo->scale(contact.major * diag, contact.minor * diag);
		cairo->begin_new_sub_path();
		cairo->arc(0, 0, 1, 0, 2 * math::num<f64>::pi);

		cairo->restore();
		cairo->stroke();
	}
}

} // namespace iptsd::gfx
