// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_DAEMON_UINPUT_DEVICE_HPP
#define IPTSD_APPS_DAEMON_UINPUT_DEVICE_HPP

#include <common/types.hpp>
#include <core/linux/syscalls.hpp>

#include <linux/input.h>
#include <linux/uinput.h>

#include <exception>
#include <fcntl.h>
#include <string>
#include <utility>

namespace syscalls = iptsd::core::linux::syscalls;

namespace iptsd::apps::daemon {

class UinputDevice {
private:
	std::string m_name;
	u16 m_vendor = 0;
	u16 m_product = 0;
	u16 m_version = 0;

	// The file descriptor of the open uinput node.
	int m_fd;

public:
	UinputDevice() : m_fd {syscalls::open("/dev/uinput", O_WRONLY | O_NONBLOCK)} {};

	~UinputDevice()
	{
		try {
			syscalls::ioctl(m_fd, UI_DEV_DESTROY);
			syscalls::close(m_fd);
		} catch (const std::exception & /* unused */) {
			// ignored
		}
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
	void set_version(const u16 version)
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
		syscalls::ioctl(m_fd, UI_SET_EVBIT, ev);
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
		syscalls::ioctl(m_fd, UI_SET_PROPBIT, prop);
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
		syscalls::ioctl(m_fd, UI_SET_KEYBIT, key);
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

		syscalls::ioctl(m_fd, UI_ABS_SETUP, &abs);
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

		const std::string name =
			fmt::format("IPTSD Virtual {} {:04X}:{:04X}", m_name, m_vendor, m_product);

		// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
		name.copy(setup.name, name.length(), 0);

		syscalls::ioctl(m_fd, UI_DEV_SETUP, &setup);
		syscalls::ioctl(m_fd, UI_DEV_CREATE);
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

		syscalls::write(m_fd, ie);
	}
};

} // namespace iptsd::apps::daemon

#endif // IPTSD_APPS_DAEMON_UINPUT_DEVICE_HPP
