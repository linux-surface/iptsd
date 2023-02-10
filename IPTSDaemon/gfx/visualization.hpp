/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_GFX_VISUALIZATION_HPP
#define IPTSD_GFX_VISUALIZATION_HPP

#include "color.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/finder.hpp>
#include <container/image.hpp>

#include <cairomm/cairomm.h>
#include <memory>
#include <vector>

namespace iptsd::gfx {

class Visualization {
private:
	std::unique_ptr<container::Image<gfx::Argb>> data = nullptr;
	config::Config config;

public:
	Visualization(config::Config config) : config {std::move(config)}
	{
	}

	void draw_heatmap(const Cairo::RefPtr<Cairo::Context> &cairo, index2_t window,
			  const container::Image<f32> &heatmap);

	void draw_contacts(const Cairo::RefPtr<Cairo::Context> &cairo, index2_t window,
			   const std::vector<contacts::Contact> &contacts);
};

} /* namespace iptsd::gfx */

#endif /* IPTSD_GFX_VISUALIZATION_HPP */
