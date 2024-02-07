// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_DAEMON_DAEMON_HPP
#define IPTSD_APPS_DAEMON_DAEMON_HPP

#include "stylus.hpp"
#include "touch.hpp"

#include <common/types.hpp>
#include <contacts/contact.hpp>
#include <core/generic/application.hpp>
#include <core/generic/config.hpp>
#include <ipts/data.hpp>

#include <spdlog/spdlog.h>

#include <optional>
#include <vector>

namespace iptsd::apps::daemon {

class Daemon : public core::Application {
private:
	// The touchscreen device.
	TouchDevice m_touch;

	// The stylus device.
	StylusDevice m_stylus;

public:
	Daemon(const core::Config &config,
	       const core::DeviceInfo &info,
	       const std::optional<const ipts::Metadata> &metadata)
		: core::Application(config, info, metadata),
		  m_touch {config, info},
		  m_stylus {config, info} {};

	void on_start() override
	{
		if (m_config.touch_disable)
			spdlog::warn("Touchscreen is disabled!");

		if (m_config.stylus_disable)
			spdlog::warn("Stylus is disabled!");
	}

	void on_contacts(const std::vector<contacts::Contact<f64>> &contacts) override
	{
		if (m_config.touch_disable)
			return;

		// Enable the touchscreen if it was disabled by a stylus that is no longer active.
		if (m_config.touch_disable_on_stylus && !m_stylus.active() && !m_touch.enabled())
			m_touch.enable();

		m_touch.update(contacts);
	}

	void on_stylus(const ipts::StylusData &stylus) override
	{
		if (m_config.stylus_disable)
			return;

		if (m_config.touch_disable_on_stylus && m_touch.enabled())
			m_touch.disable();

		m_stylus.update(stylus);
	}
};

} // namespace iptsd::apps::daemon

#endif // IPTSD_APPS_DAEMON_DAEMON_HPP
