// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_DAEMON_CONTEXT_HPP
#define IPTSD_DAEMON_CONTEXT_HPP

#include "stylus.hpp"
#include "touch.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <ipts/parser.hpp>

#include <optional>
#include <utility>

namespace iptsd::daemon {

class Context {
public:
	// The daemon configuration..
	config::Config config;

	// The touchscreen device.
	TouchDevice touch;

	// The stylus device.
	StylusDevice stylus;

	// The data returned by the metadata HID report, if applicable.
	std::optional<const ipts::Metadata> meta;

public:
	Context(config::Config config, std::optional<const ipts::Metadata> meta)
		: config {config},
		  touch {config},
		  stylus {config},
		  meta {std::move(meta)}
	{
		const f32 angle = std::cos(config.cone_angle / 180.0f * M_PIf);
		auto cone = std::make_shared<Cone>(angle, config.cone_distance);

		this->touch.set_cone(cone);
		this->stylus.set_cone(cone);
	};
};

} // namespace iptsd::daemon

#endif // IPTSD_DAEMON_CONTEXT_HPP
