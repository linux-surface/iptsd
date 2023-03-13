// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_DAEMON_TOUCH_HPP
#define IPTSD_DAEMON_TOUCH_HPP

#include "cone.hpp"
#include "uinput-device.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/contact.hpp>
#include <contacts/finder.hpp>
#include <ipts/parser.hpp>

#include <algorithm>
#include <linux/input-event-codes.h>
#include <memory>
#include <optional>
#include <set>
#include <vector>

namespace iptsd::daemon {

class TouchDevice {
private:
	UinputDevice m_uinput {};

	// The daemon configuration.
	config::Config m_config;

	// The normalized heatmap data.
	Image<f32> m_heatmap {};

	// The contact finder.
	contacts::Finder<f32, f64> m_finder;

	// The detected contacts.
	std::vector<contacts::Contact<f32>> m_contacts {};

	// The indices of the contacts in the current frame.
	std::set<usize> m_current {};

	// The indices of the contacts in the last frame.
	std::set<usize> m_last {};

	// The difference between m_last and m_current.
	std::set<usize> m_lift {};

	// The touch rejection cone.
	std::shared_ptr<Cone> m_cone;

	// The index of the contact that is emitted through the singletouch API.
	usize m_single_index = 0;

	// Whether the device is enabled.
	bool m_enabled = true;

public:
	TouchDevice(const config::Config &config, std::shared_ptr<Cone> cone)
		: m_config {config},
		  m_finder {config.contacts()},
		  m_cone {std::move(cone)}
	{
		m_uinput.name = "IPTS Touch";
		m_uinput.vendor = config.vendor;
		m_uinput.product = config.product;

		m_uinput.set_evbit(EV_ABS);
		m_uinput.set_evbit(EV_KEY);

		m_uinput.set_propbit(INPUT_PROP_DIRECT);
		m_uinput.set_keybit(BTN_TOUCH);

		const f32 diag = std::hypot(config.width, config.height);
		const i32 res_x = gsl::narrow<i32>(std::round(IPTS_MAX_X / (config.width * 10)));
		const i32 res_y = gsl::narrow<i32>(std::round(IPTS_MAX_Y / (config.height * 10)));
		const i32 res_d = gsl::narrow<i32>(std::round(IPTS_DIAGONAL / (diag * 10)));

		m_uinput.set_absinfo(ABS_MT_SLOT, 0, IPTS_MAX_CONTACTS, 0);
		m_uinput.set_absinfo(ABS_MT_TRACKING_ID, 0, IPTS_MAX_CONTACTS, 0);
		m_uinput.set_absinfo(ABS_MT_POSITION_X, 0, IPTS_MAX_X, res_x);
		m_uinput.set_absinfo(ABS_MT_POSITION_Y, 0, IPTS_MAX_Y, res_y);
		m_uinput.set_absinfo(ABS_MT_ORIENTATION, 0, 180, 0);
		m_uinput.set_absinfo(ABS_MT_TOUCH_MAJOR, 0, IPTS_DIAGONAL, res_d);
		m_uinput.set_absinfo(ABS_MT_TOUCH_MINOR, 0, IPTS_DIAGONAL, res_d);
		m_uinput.set_absinfo(ABS_X, 0, IPTS_MAX_X, res_x);
		m_uinput.set_absinfo(ABS_Y, 0, IPTS_MAX_Y, res_y);

		m_uinput.create();
	};

	/*!
	 * Converts IPTS heatmap data into touch events and passes them to the linux kernel.
	 *
	 * @param[in] data The data received from the IPTS hardware.
	 */
	void input(const ipts::Heatmap &data)
	{
		this->normalize_and_load(data);

		// Search for contacts
		m_finder.find(m_heatmap, m_contacts);

		// If the touchscreen is disabled we still want to run contact tracking.
		if (!m_enabled)
			return;

		// Find the inputs that need to be lifted
		this->search_lifted();

		// Update the rejection cone
		this->update_cone();

		if (this->is_blocked()) {
			this->lift_all();
		} else {
			this->process_multitouch();
			this->process_singletouch();
		}

		this->sync();
	}

	/*!
	 * Disables the touchscreen and lifts all contacts.
	 */
	void disable()
	{
		m_enabled = false;

		// Lift all currently active contacts.
		this->lift_all();
		this->sync();

		m_contacts.clear();
	}

	/*!
	 * Enables the touchscreen.
	 */
	void enable()
	{
		m_enabled = true;
	}

	/*!
	 * Whether the touchscreen is disabled or enabled.
	 * @return true if the touchscreen is enabled.
	 */
	[[nodiscard]] bool enabled() const
	{
		return m_enabled;
	}

	/*!
	 * Whether the touchscreen is currently active.
	 * @return true if there are any active inputs.
	 */
	[[nodiscard]] bool active() const
	{
		return !m_current.empty();
	}

private:
	/*!
	 * Prepares IPTS heatmap data for contact detection.
	 *
	 * IPTS usually sends data that goes from 255 (no contact) to 0 (contact).
	 * For contact detection we need data that goes from 0 (no contact) to 1 (contact).
	 *
	 * @param[in] data The data to process.
	 */
	void normalize_and_load(const ipts::Heatmap &data)
	{
		const Eigen::Index rows = index_cast(data.dim.height);
		const Eigen::Index cols = index_cast(data.dim.width);

		// Make sure the heatmap buffer has the right size
		if (m_heatmap.rows() != rows || m_heatmap.cols() != cols)
			m_heatmap.conservativeResize(data.dim.height, data.dim.width);

		// Map the buffer to an Eigen container
		Eigen::Map<const Image<u8>> mapped {data.data.data(), rows, cols};

		const f32 z_min = static_cast<f32>(data.dim.z_min);
		const f32 z_max = static_cast<f32>(data.dim.z_max);

		// Normalize the heatmap to range [0, 1]
		const auto norm = (mapped.cast<f32>() - z_min) / (z_max - z_min);

		// IPTS sends inverted heatmaps
		m_heatmap = 1.0f - norm;
	}

	/*!
	 * Builds the difference between the current and the last frame.
	 * Contacts that were present in the last frame but not in this one have to be lifted.
	 */
	void search_lifted()
	{
		std::swap(m_current, m_last);

		m_current.clear();

		// Build a set of current indices
		for (const contacts::Contact<f32> &contact : m_contacts) {
			if (!contact.index.has_value())
				continue;

			m_current.insert(contact.index.value());
		}

		m_lift.clear();

		// Determine all indices that were in the last frame but not in this one
		std::set_difference(m_last.cbegin(), m_last.cend(), m_current.cbegin(),
				    m_current.cend(), std::inserter(m_lift, m_lift.begin()));
	}

	/*!
	 * Updates the palm rejection cone with the positions of all palms on the display.
	 */
	void update_cone() const
	{
		// The cone has never seen a position update, so its inactive
		if (!m_cone->alive())
			return;

		if (!m_cone->active())
			return;

		for (const contacts::Contact<f32> &contact : m_contacts) {
			if (contact.valid.value_or(true))
				continue;

			// Scale to physical coordinates
			const f32 x = contact.mean.x() * m_config.width;
			const f32 y = contact.mean.y() * m_config.height;

			m_cone->update_direction(x, y);
		}
	}

	/*!
	 * Checks if a contact is marked as invalid or if it is inside of the rejection cone.
	 *
	 * @param[in] contact The contact to check.
	 * @return Whether the contact should be lifted.
	 */
	[[nodiscard]] bool should_lift(const contacts::Contact<f32> &contact) const
	{
		// Lift invalid contacts
		if (!contact.valid.value_or(true))
			return true;

		// Scale to physical coordinates
		const f32 x = contact.mean.x() * m_config.width;
		const f32 y = contact.mean.y() * m_config.height;

		// Lift contacts that are blocked by a rejection cone
		if (m_config.touch_check_cone && m_cone->check(x, y))
			return true;

		return false;
	}

	/*!
	 * Checks if the touchscreen should be disabled because of a palm on the screen.
	 *
	 * @return true if all contacts should be lifted.
	 */
	[[nodiscard]] bool is_blocked() const
	{
		if (!m_config.touch_disable_on_palm)
			return false;

		for (const contacts::Contact<f32> &c : m_contacts) {
			if (this->should_lift(c))
				return true;
		}

		return false;
	}

	/*!
	 * Emits linux multitouch events for every detected contact.
	 */
	void process_multitouch() const
	{
		for (const contacts::Contact<f32> &contact : m_contacts) {
			// Ignore contacts without an index
			if (!contact.index.has_value())
				continue;

			// Ignore unstable changes
			if (!contact.stable.value_or(true))
				continue;

			const usize index = contact.index.value();
			m_uinput.emit(EV_ABS, ABS_MT_SLOT, gsl::narrow<i32>(index));

			if (this->should_lift(contact))
				this->lift_multitouch();
			else
				this->emit_multitouch(contact);
		}

		for (const usize &index : m_lift) {
			m_uinput.emit(EV_ABS, ABS_MT_SLOT, gsl::narrow<i32>(index));
			this->lift_multitouch();
		}
	}

	/*!
	 * Emits a lift event using the linux multitouch protocol.
	 */
	void lift_multitouch() const
	{
		m_uinput.emit(EV_ABS, ABS_MT_TRACKING_ID, -1);
	}

	/*!
	 * Emits a contact event using the linux multitouch protocol.
	 *
	 * @param[in] contact The contact to emit.
	 */
	void emit_multitouch(const contacts::Contact<f32> &contact) const
	{
		const Vector2<f32> size = contact.size;

		Vector2<f32> mean = contact.mean;
		f32 orientation = contact.orientation;

		if (m_config.invert_x)
			mean.x() = 1.0f - mean.x();

		if (m_config.invert_y)
			mean.y() = 1.0f - mean.y();

		if (m_config.invert_x != m_config.invert_y)
			orientation = 1.0f - orientation;

		const i32 index = gsl::narrow<i32>(contact.index.value_or(0));

		const i32 x = gsl::narrow<i32>(std::round(mean.x() * IPTS_MAX_X));
		const i32 y = gsl::narrow<i32>(std::round(mean.y() * IPTS_MAX_Y));

		const i32 angle = gsl::narrow<i32>(std::round(orientation * 180));
		const i32 major = gsl::narrow<i32>(std::round(size.maxCoeff() * IPTS_DIAGONAL));
		const i32 minor = gsl::narrow<i32>(std::round(size.minCoeff() * IPTS_DIAGONAL));

		m_uinput.emit(EV_ABS, ABS_MT_TRACKING_ID, index);
		m_uinput.emit(EV_ABS, ABS_MT_POSITION_X, x);
		m_uinput.emit(EV_ABS, ABS_MT_POSITION_Y, y);

		m_uinput.emit(EV_ABS, ABS_MT_ORIENTATION, angle);
		m_uinput.emit(EV_ABS, ABS_MT_TOUCH_MAJOR, major);
		m_uinput.emit(EV_ABS, ABS_MT_TOUCH_MINOR, minor);
	}

	/*!
	 * Emits linux singletouch events for every detected contact.
	 */
	void process_singletouch()
	{
		bool reset = m_lift.find(m_single_index) == m_lift.cend();

		if (!reset) {
			for (const contacts::Contact<f32> &contact : m_contacts) {
				if (contact.index != m_single_index)
					continue;

				// If the contact should be lifted select a new one.
				if (!m_enabled || this->should_lift(contact))
					break;

				// Ignore unstable changes
				if (!contact.stable.value_or(true))
					return;

				// this->emit_singletouch(contact);
				return;
			}
		}

		// this->lift_singletouch();

		if (!m_enabled)
			return;

		// If this loop is reached the contact was lifted and a new one has to be found.
		for (const contacts::Contact<f32> &contact : m_contacts) {
			if (contact.index == m_single_index)
				continue;

			if (this->should_lift(contact))
				continue;

			m_single_index = contact.index.value_or(0);
			return;
		}
	}

	/*!
	 * Emits a lift event using the linux singletouch protocol.
	 */
	void lift_singletouch() const
	{
		m_uinput.emit(EV_KEY, BTN_TOUCH, 0);
	}

	/*!
	 * Emits a contact event using the linux singletouch protocol.
	 *
	 * @param[in] contact The contact to emit.
	 */
	void emit_singletouch(const contacts::Contact<f32> &contact) const
	{
		Vector2<f32> mean = contact.mean;

		if (m_config.invert_x)
			mean.x() = 1.0f - mean.x();

		if (m_config.invert_y)
			mean.y() = 1.0f - mean.y();

		const i32 x = gsl::narrow<i32>(std::round(mean.x() * IPTS_MAX_X));
		const i32 y = gsl::narrow<i32>(std::round(mean.y() * IPTS_MAX_Y));

		m_uinput.emit(EV_KEY, BTN_TOUCH, 1);
		m_uinput.emit(EV_ABS, ABS_X, x);
		m_uinput.emit(EV_ABS, ABS_Y, y);
	}

	/*!
	 * Lifts all currently active inputs.
	 */
	void lift_all() const
	{
		for (const usize index : m_current) {
			m_uinput.emit(EV_ABS, ABS_MT_SLOT, gsl::narrow<i32>(index));
			this->lift_multitouch();
		}

		for (const usize index : m_last) {
			m_uinput.emit(EV_ABS, ABS_MT_SLOT, gsl::narrow<i32>(index));
			this->lift_multitouch();
		}

		this->lift_singletouch();
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

#endif // IPTSD_DAEMON_TOUCH_HPP
