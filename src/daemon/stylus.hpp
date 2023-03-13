// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_DAEMON_STYLUS_HPP
#define IPTSD_DAEMON_STYLUS_HPP

#include "cone.hpp"
#include "uinput-device.hpp"

#include <config/config.hpp>
#include <ipts/parser.hpp>

#include <linux/input-event-codes.h>

namespace iptsd::daemon {

class StylusDevice {
private:
	UinputDevice m_uinput {};

	// The daemon configuration.
	config::Config m_config;

	// The touch rejection cone.
	std::shared_ptr<Cone> m_cone;

	// Whether the device is enabled.
	bool m_enabled = true;

	// Whether the stylus is currently active.
	bool m_active = false;

public:
	StylusDevice(const config::Config &config, std::shared_ptr<Cone> cone)
		: m_config {config},
		  m_cone {std::move(cone)}
	{
		m_uinput.set_name("IPTS Stylus");
		m_uinput.set_vendor(config.vendor);
		m_uinput.set_product(config.product);

		m_uinput.set_evbit(EV_KEY);
		m_uinput.set_evbit(EV_ABS);

		m_uinput.set_propbit(INPUT_PROP_DIRECT);
		m_uinput.set_propbit(INPUT_PROP_POINTER);

		m_uinput.set_keybit(BTN_TOUCH);
		m_uinput.set_keybit(BTN_STYLUS);
		m_uinput.set_keybit(BTN_TOOL_PEN);
		m_uinput.set_keybit(BTN_TOOL_RUBBER);

		const i32 res_x = gsl::narrow<i32>(std::round(IPTS_MAX_X / (config.width * 10)));
		const i32 res_y = gsl::narrow<i32>(std::round(IPTS_MAX_Y / (config.height * 10)));

		// Resolution for tilt is expected to be units/radian.
		const i32 res_tilt = gsl::narrow<i32>(std::round(18000.0f / M_PIf));

		m_uinput.set_absinfo(ABS_X, 0, IPTS_MAX_X, res_x);
		m_uinput.set_absinfo(ABS_Y, 0, IPTS_MAX_Y, res_y);
		m_uinput.set_absinfo(ABS_PRESSURE, 0, IPTS_MAX_PRESSURE, 0);
		m_uinput.set_absinfo(ABS_TILT_X, -9000, 9000, res_tilt);
		m_uinput.set_absinfo(ABS_TILT_Y, -9000, 9000, res_tilt);
		m_uinput.set_absinfo(ABS_MISC, 0, USHRT_MAX, 0);

		m_uinput.create();
	}

	/*!
	 * Passes IPTS stylus data to the linux kernel.
	 *
	 * @param[in] data The data received from the IPTS hardware.
	 */
	void input(const ipts::StylusData &data)
	{
		m_active = data.proximity;

		if (m_active) {
			// Scale to physical coordinates
			const f32 x = (static_cast<f32>(data.x) / IPTS_MAX_X) * m_config.width;
			const f32 y = (static_cast<f32>(data.y) / IPTS_MAX_Y) * m_config.height;

			// Update rejection cone
			m_cone->update_position(x, y);

			const bool btn_pen = data.proximity && !data.rubber;
			const bool btn_rubber = data.proximity && data.rubber;

			const Vector2<i32> tilt = this->calculate_tilt(data.altitude, data.azimuth);

			m_uinput.emit(EV_KEY, BTN_TOUCH, data.contact);
			m_uinput.emit(EV_KEY, BTN_TOOL_PEN, btn_pen);
			m_uinput.emit(EV_KEY, BTN_TOOL_RUBBER, btn_rubber);
			m_uinput.emit(EV_KEY, BTN_STYLUS, data.button);

			m_uinput.emit(EV_ABS, ABS_X, data.x);
			m_uinput.emit(EV_ABS, ABS_Y, data.y);
			m_uinput.emit(EV_ABS, ABS_PRESSURE, data.pressure);
			m_uinput.emit(EV_ABS, ABS_MISC, data.timestamp);

			m_uinput.emit(EV_ABS, ABS_TILT_X, tilt.x());
			m_uinput.emit(EV_ABS, ABS_TILT_Y, tilt.y());
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
	 * @return true if the stylus is enabled.
	 */
	[[nodiscard]] bool enabled() const
	{
		return m_enabled;
	}

	/*!
	 * Whether the stylus is currently active.
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
	[[nodiscard]] Vector2<i32> calculate_tilt(u32 altitude, u32 azimuth) const
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
		m_uinput.emit(EV_KEY, BTN_TOUCH, 0);
		m_uinput.emit(EV_KEY, BTN_TOOL_PEN, 0);
		m_uinput.emit(EV_KEY, BTN_TOOL_RUBBER, 0);
		m_uinput.emit(EV_KEY, BTN_STYLUS, 0);
	}

	/*!
	 * Commits the emitted events to the linux kernel.
	 */
	void sync() const
	{
		m_uinput.emit(EV_SYN, SYN_REPORT, 0);
	}
};

} // namespace iptsd::daemon

#endif // IPTSD_DAEMON_STYLUS_HPP
