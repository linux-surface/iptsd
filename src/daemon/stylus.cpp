// SPDX-License-Identifier: GPL-2.0-or-later

#include "stylus.hpp"

#include "cone.hpp"
#include "config.hpp"
#include "context.hpp"
#include "devices.hpp"
#include "reader.hpp"

#include <common/types.hpp>
#include <ipts/protocol.h>

#include <cmath>
#include <linux/input-event-codes.h>
#include <tuple>

static std::tuple<i32, i32> get_tilt(u32 altitude, u32 azimuth)
{
	if (altitude <= 0)
		return std::tuple<i32, i32>(0, 0);

	f32 alt = ((f32)altitude) / 18000 * M_PI;
	f32 azm = ((f32)azimuth) / 18000 * M_PI;

	f32 sin_alt = std::sin(alt);
	f32 sin_azm = std::sin(azm);

	f32 cos_alt = std::cos(alt);
	f32 cos_azm = std::cos(azm);

	f32 atan_x = std::atan2(cos_alt, sin_alt * cos_azm);
	f32 atan_y = std::atan2(cos_alt, sin_alt * sin_azm);

	i32 tx = 9000 - (atan_x * 4500 / M_PI_4);
	i32 ty = (atan_y * 4500 / M_PI_4) - 9000;

	return std::tuple<i32, i32>(tx, ty);
}

static void update_cone(IptsdContext *iptsd, struct ipts_stylus_data_v2 data)
{
	StylusDevice *stylus = iptsd->devices->active_stylus();

	f32 x = (f32)data.x / IPTS_MAX_X;
	f32 y = (f32)data.y / IPTS_MAX_Y;

	x = x * iptsd->config->width;
	y = y * iptsd->config->height;

	stylus->cone.set_tip(x, y);
}

static void handle_data(IptsdContext *iptsd, struct ipts_stylus_data_v2 data)
{
	StylusDevice *stylus = iptsd->devices->active_stylus();

	bool prox = (data.mode & IPTS_STYLUS_REPORT_MODE_PROX) >> 0;
	bool touch = (data.mode & IPTS_STYLUS_REPORT_MODE_TOUCH) >> 1;
	bool button = (data.mode & IPTS_STYLUS_REPORT_MODE_BUTTON) >> 2;
	bool rubber = (data.mode & IPTS_STYLUS_REPORT_MODE_RUBBER) >> 3;

	bool btn_pen = prox && !rubber;
	bool btn_rubber = prox && rubber;

	if (prox)
		update_cone(iptsd, data);

	std::tuple<i32, i32> tilt = get_tilt(data.altitude, data.azimuth);

	stylus->emit(EV_KEY, BTN_TOUCH, touch);
	stylus->emit(EV_KEY, BTN_TOOL_PEN, btn_pen);
	stylus->emit(EV_KEY, BTN_TOOL_RUBBER, btn_rubber);
	stylus->emit(EV_KEY, BTN_STYLUS, button);

	stylus->emit(EV_ABS, ABS_X, data.x);
	stylus->emit(EV_ABS, ABS_Y, data.y);
	stylus->emit(EV_ABS, ABS_PRESSURE, data.pressure);
	stylus->emit(EV_ABS, ABS_MISC, data.timestamp);

	stylus->emit(EV_ABS, ABS_TILT_X, std::get<0>(tilt));
	stylus->emit(EV_ABS, ABS_TILT_Y, std::get<1>(tilt));

	stylus->emit(EV_SYN, SYN_REPORT, 0);
}

static void read_v2(IptsdContext *iptsd, u8 count)
{
	for (u8 i = 0; i < count; i++) {
		auto data = iptsd->reader->read<struct ipts_stylus_data_v2>();

		handle_data(iptsd, data);
	}
}

static void read_v1(IptsdContext *iptsd, u8 count)
{
	struct ipts_stylus_data_v2 v2;

	for (u8 i = 0; i < count; i++) {
		auto data = iptsd->reader->read<struct ipts_stylus_data_v1>();

		v2.mode = data.mode;
		v2.x = data.x;
		v2.y = data.y;
		v2.pressure = data.pressure * 4;
		v2.altitude = 0;
		v2.azimuth = 0;
		v2.timestamp = 0;

		handle_data(iptsd, v2);
	}
}

static void read_report(IptsdContext *iptsd, struct ipts_report report)
{
	auto sreport = iptsd->reader->read<struct ipts_stylus_report>();
	iptsd->devices->switch_stylus(sreport.serial);

	switch (report.type) {
	case IPTS_REPORT_TYPE_STYLUS_V1:
		read_v1(iptsd, sreport.elements);
		break;
	case IPTS_REPORT_TYPE_STYLUS_V2:
		read_v2(iptsd, sreport.elements);
		break;
	}
}

void iptsd_stylus_handle_input(IptsdContext *iptsd, struct ipts_payload_frame frame)
{
	u32 size = 0;

	while (size < frame.size) {
		auto report = iptsd->reader->read<struct ipts_report>();

		switch (report.type) {
		case IPTS_REPORT_TYPE_STYLUS_V1:
		case IPTS_REPORT_TYPE_STYLUS_V2:
			read_report(iptsd, report);
			break;
		default:
			iptsd->reader->skip(report.size);
			break;
		}

		size += report.size + sizeof(report);
	}
}
