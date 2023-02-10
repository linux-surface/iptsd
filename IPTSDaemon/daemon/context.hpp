/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_CONTEXT_HPP
#define IPTSD_DAEMON_CONTEXT_HPP

#include "devices.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <ipts/parser.hpp>

#include <optional>
#include <utility>

namespace iptsd::daemon {

class Context {
public:
	config::Config config;
	DeviceManager devices;

	std::optional<const ipts::Metadata> meta;

public:
	Context(const config::Config &config, std::optional<const ipts::Metadata> meta)
		: config {config}, devices {config}, meta {std::move(meta)} {};
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_CONTEXT_HPP */
