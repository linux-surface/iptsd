// SPDX-License-Identifier: GPL-2.0-or-later

#include "devices.hpp"

#include "uinput-device.hpp"

#include <common/types.hpp>
#include <config/config.hpp>
#include <contacts/finder.hpp>
#include <ipts/protocol.hpp>

#include <climits>
#include <cmath>
#include <cstddef>
#include <gsl/gsl>
#include <iterator>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace iptsd::daemon {

static i32 res(i32 virt, f64 phys)
{
	// The kernel expects the resolution of an axis in units/mm.
	// We store width and height in centimeters, so they need to be converted.

	f64 res = virt / (phys * 10.0);
	return gsl::narrow<i32>(std::round(res));
}

StylusDevice::StylusDevice(const config::Config &conf, u32 serial, std::shared_ptr<Cone> cone)
	: UinputDevice(), serial(serial), cone(std::move(cone))
{
	this->name = "IPTS Stylus";
	this->vendor = conf.vendor;
	this->product = conf.product;

	this->set_evbit(EV_KEY);
	this->set_evbit(EV_ABS);

	this->set_propbit(INPUT_PROP_DIRECT);
	this->set_propbit(INPUT_PROP_POINTER);

	this->set_keybit(BTN_TOUCH);
	this->set_keybit(BTN_STYLUS);
	this->set_keybit(BTN_TOOL_PEN);
	this->set_keybit(BTN_TOOL_RUBBER);

	i32 res_x = res(IPTS_MAX_X, conf.width);
	i32 res_y = res(IPTS_MAX_Y, conf.height);

	// Resolution for tilt is expected to be units/radian.
	i32 res_tilt = gsl::narrow<i32>(std::round(18000 / math::num<f32>::pi));

	this->set_absinfo(ABS_X, 0, IPTS_MAX_X, res_x);
	this->set_absinfo(ABS_Y, 0, IPTS_MAX_Y, res_y);
	this->set_absinfo(ABS_PRESSURE, 0, IPTS_MAX_PRESSURE, 0);
	this->set_absinfo(ABS_TILT_X, -9000, 9000, res_tilt);
	this->set_absinfo(ABS_TILT_Y, -9000, 9000, res_tilt);
	this->set_absinfo(ABS_MISC, 0, USHRT_MAX, 0);

	this->create();
}

TouchDevice::TouchDevice(const config::Config &conf)
	: UinputDevice(), cones {}, finder {conf.contacts()}
{
	this->name = "IPTS Touch";
	this->vendor = conf.vendor;
	this->product = conf.product;

	this->set_evbit(EV_ABS);
	this->set_evbit(EV_KEY);

	this->set_propbit(INPUT_PROP_DIRECT);
	this->set_keybit(BTN_TOUCH);

	f64 diag = std::hypot(conf.width, conf.height);
	i32 res_x = res(IPTS_MAX_X, conf.width);
	i32 res_y = res(IPTS_MAX_Y, conf.height);
	i32 res_d = res(IPTS_DIAGONAL, diag);

	this->set_absinfo(ABS_MT_SLOT, 0, IPTS_MAX_CONTACTS, 0);
	this->set_absinfo(ABS_MT_TRACKING_ID, 0, IPTS_MAX_CONTACTS, 0);
	this->set_absinfo(ABS_MT_POSITION_X, 0, IPTS_MAX_X, res_x);
	this->set_absinfo(ABS_MT_POSITION_Y, 0, IPTS_MAX_Y, res_y);
	this->set_absinfo(ABS_MT_ORIENTATION, 0, 180, 0);
	this->set_absinfo(ABS_MT_TOUCH_MAJOR, 0, IPTS_DIAGONAL, res_d);
	this->set_absinfo(ABS_MT_TOUCH_MINOR, 0, IPTS_DIAGONAL, res_d);
	this->set_absinfo(ABS_X, 0, IPTS_MAX_X, res_x);
	this->set_absinfo(ABS_Y, 0, IPTS_MAX_Y, res_y);

	this->create();
}

DeviceManager::DeviceManager(const config::Config &conf) : conf {conf}, touch {conf}
{
	this->create_stylus(0);
}

StylusDevice &DeviceManager::create_stylus(u32 serial)
{
	std::shared_ptr<Cone> cone = std::make_shared<Cone>(conf.cone_angle, conf.cone_distance);
	this->touch.cones.push_back(cone);
	return this->styli.emplace_back(this->conf, serial, std::move(cone));
}

StylusDevice &DeviceManager::get_stylus(u32 serial)
{
	StylusDevice &stylus = this->styli.back();

	if (stylus.serial == serial)
		return stylus;

	if (stylus.serial == 0) {
		stylus.serial = serial;
		return stylus;
	}

	for (StylusDevice &s : this->styli) {
		if (s.serial != serial)
			continue;

		std::swap(s, stylus);
		return s;
	}

	return this->create_stylus(serial);
}

} // namespace iptsd::daemon
