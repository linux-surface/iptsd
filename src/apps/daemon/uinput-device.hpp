// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_DAEMON_UINPUT_DEVICE_HPP
#define IPTSD_APPS_DAEMON_UINPUT_DEVICE_HPP

#include <common/cerror.hpp>
#include <common/cwrap.hpp>
#include <common/types.hpp>

#include <linux/uinput.h>
#include <string>

namespace iptsd::apps::daemon {

class UinputDevice {
private:
	std::string m_name = "";
	u16 m_vendor = 0;
	u16 m_product = 0;
	u32 m_version = 0;

	// The file descriptor of the open uinput node.
	int m_fd;

public:
	UinputDevice()
	{
		const int ret = common::open("/dev/uinput", O_WRONLY | O_NONBLOCK);
		if (ret == -1)
			throw common::cerror("Failed to open uinput device");

		m_fd = ret;
	}

	~UinputDevice()
	{
		common::ioctl(m_fd, UI_DEV_DESTROY);
		close(m_fd);
	}

	/*!
	 * Sets the name of the device.
	 *
	 * Must be called before @ref create().
	 *
	 * @param[in] name The new name.
	 */
	void set_name(std::string name)
	{
		m_name = std::move(name);
	}

	/*!
	 * Sets the vendor ID of the device.
	 *
	 * Must be called before @ref create().
	 *
	 * @param[in] vendor The vendor ID.
	 */
	void set_vendor(const u16 vendor)
	{
		m_vendor = vendor;
	}

	/*!
	 * Sets the product ID of the device.
	 *
	 * Must be called before @ref create().
	 *
	 * @param[in] product The product ID.
	 */
	void set_product(const u16 product)
	{
		m_product = product;
	}

	/*!
	 * Sets the version number of the device.
	 *
	 * Must be called before @ref create().
	 *
	 * @param[in] version The firmware or hardware revision.
	 */
	void set_version(const u32 version)
	{
		m_version = version;
	}

	/*!
	 * Enables an event type for this device.
	 *
	 * Must be called before @ref create().
	 *
	 * @param[in] ev The event type to enable (e.g. EV_KEY or EV_ABS).
	 */
	void set_evbit(const i32 ev) const
	{
		const int ret = common::ioctl(m_fd, UI_SET_EVBIT, ev);
		if (ret == -1)
			throw common::cerror("UI_SET_EVBIT failed");
	}

	/*!
	 * Sets a property of the device.
	 *
	 * Must be called before @ref create().
	 *
	 * @param[in] prop The property to enable (e.g. INPUT_PROP_POINTER).
	 */
	void set_propbit(const i32 prop) const
	{
		const int ret = common::ioctl(m_fd, UI_SET_PROPBIT, prop);
		if (ret == -1)
			throw common::cerror("UI_SET_PROPBIT failed");
	}

	/*!
	 * Enables a key event for this device.
	 *
	 * Must be called before @ref create().
	 *
	 * @param[in] key They key to enable (e.g. BTN_TOUCH).
	 */
	void set_keybit(const i32 key) const
	{
		const int ret = common::ioctl(m_fd, UI_SET_KEYBIT, key);
		if (ret == -1)
			throw common::cerror("UI_SET_KEYBIT failed");
	}

	/*!
	 * Enables an axis event for this device.
	 *
	 * Must be called before @ref create().
	 *
	 * @param[in] code The event to enable (e.g. ABS_X).
	 * @param[in] min The minimal value of the axis.
	 * @param[in] max The maximal value of the axis.
	 * @param[in] res The resolution of the axis, for converting virtual to physical units.
	 */
	void set_absinfo(const u16 code, const i32 min, const i32 max, const i32 res) const
	{
		struct uinput_abs_setup abs {};

		abs.code = code;
		abs.absinfo.minimum = min;
		abs.absinfo.maximum = max;
		abs.absinfo.resolution = res;

		const int ret = iptsd::common::ioctl(m_fd, UI_ABS_SETUP, &abs);
		if (ret == -1)
			throw common::cerror("UI_ABS_SETUP failed");
	}

	/*!
	 * Finalizes the device creation.
	 */
	void create() const
	{
		struct uinput_setup setup {};

		setup.id.bustype = BUS_VIRTUAL;
		setup.id.vendor = m_vendor;
		setup.id.product = m_product;
		setup.id.version = m_version;

		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
		m_name.copy(setup.name, m_name.length(), 0);

		int ret = common::ioctl(m_fd, UI_DEV_SETUP, &setup);
		if (ret == -1)
			throw common::cerror("UI_DEV_SETUP failed");

		ret = iptsd::common::ioctl(m_fd, UI_DEV_CREATE);
		if (ret == -1)
			throw common::cerror("UI_DEV_CREATE failed");
	}

	/*!
	 * Emits an event.
	 *
	 * Must be called after @ref create().
	 *
	 * @param[in] type The event type.
	 * @param[in] key The key of the button or axis.
	 * @param[in] value The value of the button or axis.
	 */
	void emit(const u16 type, const u16 key, const i32 value) const
	{
		struct input_event ie {};

		ie.type = type;
		ie.code = key;
		ie.value = value;

		const ssize_t ret = write(m_fd, &ie, sizeof(ie));
		if (ret == -1)
			throw common::cerror("Failed to write input event");
	}
};

} // namespace iptsd::apps::daemon

#endif // IPTSD_APPS_DAEMON_UINPUT_DEVICE_HPP
