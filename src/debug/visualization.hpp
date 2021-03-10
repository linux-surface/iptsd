#pragma once

#include "gfx/cairo.hpp"
#include "gfx/color.hpp"

#include <common/types.hpp>
#include <contacts/processor.hpp>
#include <container/image.hpp>

#include <vector>

namespace iptsd {

class Visualization {
public:
	Visualization(index2_t heatmap_size) : m_data {heatmap_size} {};

	void draw(gfx::cairo::Cairo &cr, container::Image<f32> const &img,
		  std::vector<contacts::TouchPoint> const &tps, int width, int height);

private:
	container::Image<gfx::Srgb> m_data;
};

} /* namespace iptsd */
