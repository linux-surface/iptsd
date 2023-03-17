// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_DAEMON_STYLUS_HPP
#define IPTSD_APPS_DAEMON_STYLUS_HPP

#include "uinput-device.hpp"

#include <core/generic/config.hpp>
#include <core/generic/device.hpp>
#include <ipts/parser.hpp>

#include <linux/input-event-codes.h>
#include <memory>

namespace iptsd::apps::daemon {

class StylusDevice {
private:
	std::shared_ptr<UinputDevice> m_uinput = std::make_shared<UinputDevice>();

	// Whether the device is enabled.
	bool m_enabled = true;

	// Whether the stylus is currently in proximity and sending data.
	bool m_active = false;

public:
	StylusDevice(const core::Config &config, const core::DeviceInfo &info)
	{
		m_uinput->set_name("IPTS Stylus");
		m_uinput->set_vendor(info.vendor);
		m_uinput->set_product(info.product);

		m_uinput->set_evbit(EV_KEY);
		m_uinput->set_evbit(EV_ABS);

		m_uinput->set_propbit(INPUT_PROP_DIRECT);
		m_uinput->set_propbit(INPUT_PROP_POINTER);

		m_uinput->set_keybit(BTN_TOUCH);
		m_uinput->set_keybit(BTN_STYLUS);
		m_uinput->set_keybit(BTN_TOOL_PEN);
		m_uinput->set_keybit(BTN_TOOL_RUBBER);

		// Resolution for X / Y is expected to be units/mm.
		const i32 res_x = gsl::narrow<i32>(std::round(IPTS_MAX_X / (config.width * 10)));
		const i32 res_y = gsl::narrow<i32>(std::round(IPTS_MAX_Y / (config.height * 10)));

		// Resolution for tilt is expected to be units/radian.
		const i32 res_tilt = gsl::narrow<i32>(std::round(18000.0f / M_PIf));

		m_uinput->set_absinfo(ABS_X, 0, IPTS_MAX_X, res_x);
		m_uinput->set_absinfo(ABS_Y, 0, IPTS_MAX_Y, res_y);
		m_uinput->set_absinfo(ABS_PRESSURE, 0, IPTS_MAX_PRESSURE, 0);
		m_uinput->set_absinfo(ABS_TILT_X, -9000, 9000, res_tilt);
		m_uinput->set_absinfo(ABS_TILT_Y, -9000, 9000, res_tilt);
		m_uinput->set_absinfo(ABS_MISC, 0, USHRT_MAX, 0);

		m_uinput->create();
	}

	/*!
	 * Passes stylus data to the linux kernel.
	 *
	 * @param[in] data The current state of the stylus.
	 */
	void update(const ipts::StylusData &data)
	{
		m_active = data.proximity;

		if (m_active) {
			const Vector2<i32> tilt = this->calculate_tilt(data.altitude, data.azimuth);

			m_uinput->emit(EV_KEY, BTN_TOUCH, data.contact);
			m_uinput->emit(EV_KEY, BTN_TOOL_PEN, !data.rubber);
			m_uinput->emit(EV_KEY, BTN_TOOL_RUBBER, data.rubber);
			m_uinput->emit(EV_KEY, BTN_STYLUS, data.button);

			m_uinput->emit(EV_ABS, ABS_X, data.x);
			m_uinput->emit(EV_ABS, ABS_Y, data.y);
			m_uinput->emit(EV_ABS, ABS_PRESSURE, data.pressure);
			m_uinput->emit(EV_ABS, ABS_MISC, data.timestamp);

			m_uinput->emit(EV_ABS, ABS_TILT_X, tilt.x());
			m_uinput->emit(EV_ABS, ABS_TILT_Y, tilt.y());
		} else {
			this->lift();
		}

		this->sync();
	}

	/*!
	 * Disables and lifts the stylus.
	 */
	void disable()
	{
		m_enabled = false;
		m_active = false;

		// Lift all currently active contacts.
		this->lift();
		this->sync();
	}

	/*!
	 * Enables the stylus.
	 */
	void enable()
	{
		m_enabled = true;
	}

	/*!
	 * Whether the stylus is disabled or enabled.
	 *
	 * @return true if the stylus is enabled.
	 */
	[[nodiscard]] bool enabled() const
	{
		return m_enabled;
	}

	/*!
	 * Whether the stylus is currently active.
	 *
	 * @return true if the stylus is in proximity.
	 */
	[[nodiscard]] bool active() const
	{
		return m_active;
	}

private:
	/*!
	 * Calculates the tilt of the stylus on X and Y axis.
	 *
	 * @param[in] altitude The altitude of the stylus.
	 * @param[in] azimuth The azimuth of the stylus.
	 * @return A Vector containing the tilt on the X and Y axis.
	 */
	[[nodiscard]] Vector2<i32> calculate_tilt(const u32 altitude, const u32 azimuth) const
	{
		if (altitude <= 0)
			return Vector2<i32>::Zero();

		const f32 alt = static_cast<f32>(altitude) / 18000.0f * M_PIf;
		const f32 azm = static_cast<f32>(azimuth) / 18000.0f * M_PIf;

		const f32 sin_alt = std::sin(alt);
		const f32 sin_azm = std::sin(azm);

		const f32 cos_alt = std::cos(alt);
		const f32 cos_azm = std::cos(azm);

		const f32 atan_x = std::atan2(cos_alt, sin_alt * cos_azm);
		const f32 atan_y = std::atan2(cos_alt, sin_alt * sin_azm);

		const i32 tx = 9000 - gsl::narrow<i32>(std::round(atan_x * 4500 / M_PI_4f));
		const i32 ty = gsl::narrow<i32>(std::round(atan_y * 4500 / M_PI_4f)) - 9000;

		return Vector2<i32> {tx, ty};
	}

	/*!
	 * Lifts the stylus input.
	 */
	void lift() const
	{
		m_uinput->emit(EV_KEY, BTN_TOUCH, 0);
		m_uinput->emit(EV_KEY, BTN_TOOL_PEN, 0);
		m_uinput->emit(EV_KEY, BTN_TOOL_RUBBER, 0);
		m_uinput->emit(EV_KEY, BTN_STYLUS, 0);
	}

	/*!
	 * Commits the emitted events to the linux kernel.
	 */
	void sync() const
	{
		m_uinput->emit(EV_SYN, SYN_REPORT, 0);
	}
};

} // namespace iptsd::apps::daemon

#endif // IPTSD_APPS_DAEMON_STYLUS_HPP
