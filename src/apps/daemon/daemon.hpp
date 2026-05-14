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
	// Put this enum here as this class is the only one that needs to quickly reference it
	enum class DisableOnStylus : u8 {
		Off = 0,
		Connected,
		Active,
		Contact,
	};

private:
	// The touch device.
	std::optional<TouchDevice> m_touch = std::nullopt;

	// The stylus device.
	std::optional<StylusDevice> m_stylus = std::nullopt;

	// The state of the config setting Touchscreen.DisableOnStylus
	DisableOnStylus m_disable_on_stylus = DisableOnStylus::Connected;

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

		if (m_config.touchscreen_disable_on_stylus == "active")
			m_disable_on_stylus = DisableOnStylus::Active;
		else if (m_config.touchscreen_disable_on_stylus == "contact")
			m_disable_on_stylus = DisableOnStylus::Contact;
		else if (m_config.touchscreen_disable_on_stylus == "true" ||
		         m_config.touchscreen_disable_on_stylus == "connected")
			m_disable_on_stylus = DisableOnStylus::Connected;
		else
			m_disable_on_stylus = DisableOnStylus::Off;
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
		if (m_disable_on_stylus != DisableOnStylus::Off && m_stylus.has_value()) {
			if ((m_disable_on_stylus == DisableOnStylus::Active &&
			     !m_stylus->active()) ||
			    (m_disable_on_stylus == DisableOnStylus::Contact &&
			     !m_stylus->contact()) ||
			    // stylus cant report a state after its disconnected (obviously), so
			    // just check if its active for reenabling after connection (this was
			    // the old behavior)
			    (m_disable_on_stylus == DisableOnStylus::Connected &&
			     !m_stylus->active()))
				if (!m_touch->enabled())
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

		if (m_disable_on_stylus != DisableOnStylus::Off && m_touch.has_value()) {
			if ((m_disable_on_stylus == DisableOnStylus::Active &&
			     m_stylus->active()) ||
			    (m_disable_on_stylus == DisableOnStylus::Contact &&
			     m_stylus->contact()) ||
			    (m_disable_on_stylus == DisableOnStylus::Connected))
				if (m_touch->enabled())
					m_touch->disable();
		}

		m_stylus->update(stylus);
	}
};

} // namespace iptsd::apps::daemon

#endif // IPTSD_APPS_DAEMON_DAEMON_HPP
