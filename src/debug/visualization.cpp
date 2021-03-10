#include "visualization.hpp"

#include "gfx/cairo.hpp"

#include <contacts/processor.hpp>
#include <container/image.hpp>

namespace iptsd {

void Visualization::draw(gfx::cairo::Cairo &cr, container::Image<f32> const &img,
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
	gfx::cmap::cubehelix(0.1, -0.6, 1.0, 2.0).map_into(m_data, img, {{0.1f, 0.7f}});

	// create source surface based on data
	auto src = gfx::cairo::image_surface_create(m_data);

	// select font
	cr.select_font_face("monospace", gfx::cairo::FontSlant::Normal,
			    gfx::cairo::FontWeight::Normal);
	cr.set_font_size(12.0);

	// plot heatmap
	auto m = gfx::cairo::Matrix::identity();
	m.translate({0.0, img_h});
	m.scale({img_w / win_w, -img_h / win_h});

	auto p = gfx::cairo::Pattern::create_for_surface(src);
	p.set_matrix(m);
	p.set_filter(gfx::cairo::Filter::Nearest);

	cr.set_source(p);
	cr.rectangle({0, 0}, {win_w, win_h});
	cr.fill();

	auto txtbuf = std::array<char, 32> {};

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
		cr.set_source(gfx::Srgba {0.0, 0.0, 0.0, 0.33});

		cr.move_to(t({tp.mean.x + 0.5, tp.mean.y + 0.5}));
		cr.line_to(t({tp.mean.x + 0.5 + v1.x, tp.mean.y + 0.5 + v1.y}));

		cr.move_to(t({tp.mean.x + 0.5, tp.mean.y + 0.5}));
		cr.line_to(t({tp.mean.x + 0.5 + v2.x, tp.mean.y + 0.5 + v2.y}));

		cr.stroke();

		// mean
		cr.set_source(gfx::Srgb {1.0, 0.0, 0.0});

		cr.move_to(t({tp.mean.x + 0.1, tp.mean.y + 0.5}));
		cr.line_to(t({tp.mean.x + 0.9, tp.mean.y + 0.5}));

		cr.move_to(t({tp.mean.x + 0.5, tp.mean.y + 0.1}));
		cr.line_to(t({tp.mean.x + 0.5, tp.mean.y + 0.9}));

		cr.stroke();

		// standard deviation ellipse
		cr.set_source(gfx::Srgb {1.0, 0.0, 0.0});

		cr.save();

		cr.translate(t({tp.mean.x + 0.5, tp.mean.y + 0.5}));
		cr.rotate(std::atan2(v1.x, v1.y));
		cr.scale({s2 * win_w / img_w, s1 * win_h / img_h});
		cr.arc({0.0, 0.0}, 1.0, 0.0, 2.0 * math::num<f64>::pi);

		cr.restore();
		cr.stroke();

		// stats
		cr.set_source(gfx::Srgb {1.0, 1.0, 1.0});

		std::snprintf(txtbuf.data(), txtbuf.size(), "c:%.02f", tp.confidence);
		cr.move_to(t({tp.mean.x - 3.5, tp.mean.y + 3.0}));
		cr.show_text(txtbuf.data());

		std::snprintf(txtbuf.data(), txtbuf.size(), "a:%.02f",
			      std::max(s1, s2) / std::min(s1, s2));
		cr.move_to(t({tp.mean.x - 3.5, tp.mean.y + 2.0}));
		cr.show_text(txtbuf.data());

		std::snprintf(txtbuf.data(), txtbuf.size(), "s:%.02f", tp.scale);
		cr.move_to(t({tp.mean.x - 3.5, tp.mean.y + 1.0}));
		cr.show_text(txtbuf.data());
	}
}

} /* namespace iptsd */
