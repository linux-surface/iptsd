// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_DAEMON_DAEMON_HPP
#define IPTSD_APPS_DAEMON_DAEMON_HPP

#include "stylus.hpp"
#include "touch.hpp"

#include <common/types.hpp>
#include <contacts/contact.hpp>
#include <core/generic/application.hpp>
#include <core/generic/config.hpp>
#include <ipts/samples/button.hpp>
#include <ipts/samples/stylus.hpp>

#include <spdlog/spdlog.h>

#include <vector>

namespace iptsd::apps::daemon {

class Daemon : public core::Application {
private:
	// The touch device.
	std::optional<TouchDevice> m_touch = std::nullopt;

	// The stylus device.
	std::optional<StylusDevice> m_stylus = std::nullopt;

public:
	Daemon(const core::Config &config, const core::DeviceInfo &info)
		: core::Application(config, info)
	{
		const bool create_touch =
			(m_info.is_touchscreen() && !m_config.touchscreen_disable) ||
			(m_info.is_touchpad() && !m_config.touchpad_disable);

		if (create_touch)
			m_touch.emplace(config, info);

		if (m_info.is_touchscreen() && !m_config.stylus_disable)
			m_stylus.emplace(config, info);
	}

	void on_start() override
	{
		if (!m_touch.has_value() && m_info.is_touchscreen())
			spdlog::warn("Touchscreen is disabled!");

		if (!m_touch.has_value() && m_info.is_touchpad())
			spdlog::warn("Touchpad is disabled!");

		if (!m_stylus.has_value())
			spdlog::warn("Stylus is disabled!");
	}

	void on_touch(const std::vector<contacts::Contact<f64>> &contacts) override
	{
		if (!m_touch.has_value())
			return;

		// Enable the touchscreen if it was disabled by a stylus that is no longer active.
		if (m_config.touchscreen_disable_on_stylus && m_stylus.has_value()) {
			if (!m_stylus->active() && !m_touch->enabled())
				m_touch->enable();
		}

		m_touch->update(contacts);
	}

	void on_button(const ipts::samples::Button &button) override
	{
		if (!m_touch.has_value())
			return;

		m_touch->update(button);
	}

	void on_stylus(const ipts::samples::Stylus &stylus) override
	{
		if (!m_stylus.has_value())
			return;

		if (m_config.touchscreen_disable_on_stylus && m_touch.has_value()) {
			if (m_touch->enabled())
				m_touch->disable();
		}

		m_stylus->update(stylus);
	}
};

} // namespace iptsd::apps::daemon

#endif // IPTSD_APPS_DAEMON_DAEMON_HPP
