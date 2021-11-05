// SPDX-License-Identifier: GPL-2.0-or-later

#include "visualization.hpp"

#include "cmap.hpp"

#include <contacts/advanced/processor.hpp>
#include <container/image.hpp>

#include <cairo.h>
#include <cairomm/cairomm.h>
#include <cairomm/context.h>
#include <cairomm/enums.h>
#include <cairomm/matrix.h>
#include <cairomm/pattern.h>
#include <cairomm/refptr.h>
#include <cairomm/surface.h>

namespace iptsd::gfx {

Cairo::RefPtr<Cairo::ImageSurface> image_surface_create(container::Image<Argb> &image)
{
	auto const format = Cairo::FORMAT_ARGB32;
	auto const size = image.size();
	auto const data = reinterpret_cast<u8 *>(image.data());

	auto const stride = Cairo::ImageSurface::format_stride_for_width(format, size.x);
	return Cairo::ImageSurface::create(data, format, size.x, size.y, stride);
}

inline void move_to(const Cairo::RefPtr<Cairo::Context> &cr, math::Vec2<f64> v)
{
	cr->move_to(v.x, v.y);
}

inline void line_to(const Cairo::RefPtr<Cairo::Context> &cr, math::Vec2<f64> v)
{
	cr->line_to(v.x, v.y);
}

inline void translate(const Cairo::RefPtr<Cairo::Context> &cr, math::Vec2<f64> v)
{
	cr->translate(v.x, v.y);
}

void Visualization::draw(const Cairo::RefPtr<Cairo::Context> &cr, container::Image<f32> const &img,
			 std::vector<contacts::TouchPoint> const &tps, int width, int height)
{
	auto const img_w = static_cast<f64>(img.size().x);
	auto const img_h = static_cast<f64>(img.size().y);

	auto const win_w = static_cast<f64>(width);
	auto const win_h = static_cast<f64>(height);

	auto const t = [&](math::Vec2<f64> p) -> math::Vec2<f64> {
		return {p.x * (win_w / img_w), win_h - p.y * (win_h / img_h)};
	};

	// plot
	cmap::cubehelix(0.1, -0.6, 1.0, 2.0).map_into(m_data, img, {{0.1f, 0.7f}});

	// create source surface based on data
	auto src = image_surface_create(m_data);

	// select font
	cr->select_font_face("monospace", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
	cr->set_font_size(12.0);

	// plot heatmap
	auto m = Cairo::identity_matrix();
	m.translate(0.0, img_h);
	m.scale(img_w / win_w, -img_h / win_h);

	auto p = Cairo::SurfacePattern::create(src);
	p->set_matrix(m);
	p->set_filter(Cairo::FILTER_NEAREST);

	cr->set_source(p);
	cr->rectangle(0, 0, win_w, win_h);
	cr->fill();

	// plot touch-points
	for (auto const &tp : tps) {
		auto const eigen = tp.cov.eigen();

		// get standard deviation
		auto const nstd = 1.0;
		auto const s1 = nstd * std::sqrt(eigen.w[0]);
		auto const s2 = nstd * std::sqrt(eigen.w[1]);

		// eigenvectors scaled with standard deviation
		auto const v1 = eigen.v[0].cast<f64>() * s1;
		auto const v2 = eigen.v[1].cast<f64>() * s2;

		// standard deviation
		cr->set_source_rgba(0.0, 0.0, 0.0, 0.33);

		move_to(cr, t({tp.mean.x + 0.5, tp.mean.y + 0.5}));
		line_to(cr, t({tp.mean.x + 0.5 + v1.x, tp.mean.y + 0.5 + v1.y}));

		move_to(cr, t({tp.mean.x + 0.5, tp.mean.y + 0.5}));
		line_to(cr, t({tp.mean.x + 0.5 + v2.x, tp.mean.y + 0.5 + v2.y}));

		cr->stroke();

		// mean
		cr->set_source_rgb(1.0, 0.0, 0.0);

		move_to(cr, t({tp.mean.x + 0.1, tp.mean.y + 0.5}));
		line_to(cr, t({tp.mean.x + 0.9, tp.mean.y + 0.5}));

		move_to(cr, t({tp.mean.x + 0.5, tp.mean.y + 0.1}));
		line_to(cr, t({tp.mean.x + 0.5, tp.mean.y + 0.9}));

		cr->stroke();

		// standard deviation ellipse
		cr->set_source_rgb(1.0, 0.0, 0.0);

		cr->save();

		translate(cr, t({tp.mean.x + 0.5, tp.mean.y + 0.5}));
		cr->rotate(std::atan2(v1.x, v1.y));
		cr->scale(s2 * win_w / img_w, s1 * win_h / img_h);
		cr->arc(0.0, 0.0, 1.0, 0.0, 2.0 * math::num<f64>::pi);

		cr->restore();
		cr->stroke();

		// stats
		cr->set_source_rgb(1.0, 1.0, 1.0);

		move_to(cr, t({tp.mean.x - 3.5, tp.mean.y + 3.0}));
		cr->show_text(fmt::format("c:{:.02f}", tp.confidence));

		move_to(cr, t({tp.mean.x - 3.5, tp.mean.y + 2.0}));
		cr->show_text(fmt::format("a:{:.02f}", std::max(s1, s2) / std::min(s1, s2)));

		move_to(cr, t({tp.mean.x - 3.5, tp.mean.y + 1.0}));
		cr->show_text(fmt::format("s:{:.02f}", tp.scale));
	}
}

} // namespace iptsd::gfx
