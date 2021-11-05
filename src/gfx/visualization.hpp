/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_GFX_VISUALIZATION_HPP
#define IPTSD_GFX_VISUALIZATION_HPP

#include "color.hpp"

#include <common/types.hpp>
#include <contacts/advanced/processor.hpp>
#include <container/image.hpp>

#include <cairomm/context.h>
#include <cairomm/refptr.h>
#include <vector>

namespace iptsd::gfx {

class Visualization {
public:
	Visualization(index2_t heatmap_size) : m_data {heatmap_size} {};

	void draw(const Cairo::RefPtr<Cairo::Context> &cr, container::Image<f32> const &img,
		  std::vector<contacts::TouchPoint> const &tps, int width, int height);

private:
	container::Image<gfx::Argb> m_data;
};

} /* namespace iptsd::gfx */

#endif /* IPTSD_GFX_VISUALIZATION_HPP */
