// SPDX-License-Identifier: GPL-2.0-or-later

#include "devices.hpp"

#include "cone.hpp"
#include "config.hpp"
#include "heatmap.hpp"
#include "touch-processing.hpp"
#include "uinput-device.hpp"

#include <common/types.hpp>
#include <ipts/ipts.h>
#include <ipts/protocol.h>

#include <climits>
#include <cmath>
#include <iterator>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdexcept>
#include <string>

static i32 res(i32 virt, i32 phys)
{
	f64 res = (f64)(virt * 10) / (f64)phys;
	return (i32)std::round(res);
}

StylusDevice::StylusDevice(struct ipts_device_info info, IptsdConfig *conf) : UinputDevice()
{
	this->name = "IPTS Stylus";
	this->vendor = info.vendor;
	this->product = info.product;
	this->version = info.version;

	this->set_evbit(EV_KEY);
	this->set_evbit(EV_ABS);

	this->set_propbit(INPUT_PROP_DIRECT);
	this->set_propbit(INPUT_PROP_POINTER);

	this->set_keybit(BTN_TOUCH);
	this->set_keybit(BTN_STYLUS);
	this->set_keybit(BTN_TOOL_PEN);
	this->set_keybit(BTN_TOOL_RUBBER);

	i32 res_x = res(IPTS_MAX_X, conf->width);
	i32 res_y = res(IPTS_MAX_Y, conf->height);

	this->set_absinfo(ABS_X, 0, IPTS_MAX_X, res_x);
	this->set_absinfo(ABS_Y, 0, IPTS_MAX_Y, res_y);
	this->set_absinfo(ABS_PRESSURE, 0, 4096, 0);
	this->set_absinfo(ABS_TILT_X, -9000, 9000, 18000 / M_PI);
	this->set_absinfo(ABS_TILT_Y, -9000, 9000, 18000 / M_PI);
	this->set_absinfo(ABS_MISC, 0, USHRT_MAX, 0);

	this->create();
}

TouchDevice::TouchDevice(struct ipts_device_info info, IptsdConfig *conf)
	: UinputDevice(), processor(info, conf)
{
	this->conf = conf;
	this->hm = nullptr;

	this->name = "IPTS Touch";
	this->vendor = info.vendor;
	this->product = info.product;
	this->version = info.version;

	this->set_evbit(EV_ABS);
	this->set_evbit(EV_KEY);

	this->set_propbit(INPUT_PROP_DIRECT);
	this->set_keybit(BTN_TOUCH);

	f32 diag = std::sqrt(conf->width * conf->width + conf->height * conf->height);
	i32 res_x = res(IPTS_MAX_X, conf->width);
	i32 res_y = res(IPTS_MAX_Y, conf->height);
	i32 res_d = res(IPTS_DIAGONAL, diag);

	this->set_absinfo(ABS_MT_SLOT, 0, info.max_contacts, 0);
	this->set_absinfo(ABS_MT_TRACKING_ID, 0, info.max_contacts, 0);
	this->set_absinfo(ABS_MT_POSITION_X, 0, IPTS_MAX_X, res_x);
	this->set_absinfo(ABS_MT_POSITION_Y, 0, IPTS_MAX_Y, res_y);
	this->set_absinfo(ABS_MT_TOOL_TYPE, 0, MT_TOOL_MAX, 0);
	this->set_absinfo(ABS_MT_TOOL_X, 0, IPTS_MAX_X, res_x);
	this->set_absinfo(ABS_MT_TOOL_X, 0, IPTS_MAX_X, res_x);
	this->set_absinfo(ABS_MT_ORIENTATION, 0, 180, 0);
	this->set_absinfo(ABS_MT_TOUCH_MAJOR, 0, IPTS_DIAGONAL, res_d);
	this->set_absinfo(ABS_MT_TOUCH_MINOR, 0, IPTS_DIAGONAL, res_d);
	this->set_absinfo(ABS_X, 0, IPTS_MAX_X, res_x);
	this->set_absinfo(ABS_Y, 0, IPTS_MAX_Y, res_y);

	this->create();
}

Heatmap *TouchDevice::get_heatmap(i32 w, i32 h)
{
	if (this->hm && (this->hm->width != w || this->hm->height != h))
		delete this->hm;

	if (!this->hm)
		this->hm = new Heatmap(w, h, this->conf->touch_threshold);

	return this->hm;
}

DeviceManager::DeviceManager(struct ipts_device_info info, IptsdConfig *conf) : touch(info, conf)
{
	if (conf->width == 0 || conf->height == 0)
		throw std::runtime_error("Display size is 0");

	this->info = info;
	this->conf = conf;

	this->styli = std::vector<StylusDevice *>();
	this->switch_stylus(0);
}

DeviceManager::~DeviceManager(void)
{
	for (StylusDevice *stylus : this->styli)
		delete stylus;
}

StylusDevice *DeviceManager::active_stylus(void)
{
	return this->styli[this->active_index];
}

void DeviceManager::switch_stylus(u32 serial)
{
	for (size_t i = 0; i < std::size(this->styli); i++) {
		if (this->styli[i]->serial != serial)
			continue;

		this->active_index = i;
		return;
	}

	if (std::size(this->styli) > 0 && this->active_stylus()->serial == 0) {
		this->active_stylus()->serial = serial;
		return;
	}

	StylusDevice *stylus = new StylusDevice(this->info, this->conf);
	stylus->serial = serial;
	this->styli.insert(this->styli.end(), stylus);
	this->active_index = std::size(this->styli) - 1;

	TouchProcessor *tp = &this->touch.processor;
	tp->rejection_cones.insert(tp->rejection_cones.end(), &stylus->cone);
}
