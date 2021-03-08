#pragma once

#include "gfx/cairo.hpp"
#include "gfx/color.hpp"

#include <contacts/container/image.hpp>
#include <contacts/processor.hpp>
#include <contacts/types.hpp>

#include <vector>

namespace iptsd {

class Visualization {
public:
	Visualization(index2_t heatmap_size) : m_data {heatmap_size} {};

	void draw(gfx::cairo::Cairo &cr, Image<f32> const &img, std::vector<TouchPoint> const &tps,
		  int width, int height);

private:
	Image<gfx::Srgb> m_data;
};

} /* namespace iptsd */
