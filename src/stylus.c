// SPDX-License-Identifier: GPL-2.0-or-later

#include <math.h>
#include <linux/uinput.h>
#include <stdint.h>
#include <stdio.h>

#include "cone.h"
#include "context.h"
#include "devices.h"
#include "protocol.h"
#include "reader.h"
#include "stylus.h"
#include "utils.h"

static void iptsd_stylus_tilt(int altitude, int azimuth, int *tx, int *ty)
{
	*tx = 0;
	*ty = 0;

	if (altitude <= 0)
		return;

	double alt = ((double)altitude) / 18000 * M_PI;
	double azm = ((double)azimuth) / 18000 * M_PI;

	double sin_alt = sin(alt);
	double sin_azm = sin(azm);

	double cos_alt = cos(alt);
	double cos_azm = cos(azm);

	double atan_x = atan2(cos_alt, sin_alt * cos_azm);
	double atan_y = atan2(cos_alt, sin_alt * sin_azm);

	*tx = 9000 - (atan_x * 4500 / M_PI_4);
	*ty = (atan_y * 4500 / M_PI_4) - 9000;
}

static void iptsd_stylus_update_cone(struct iptsd_context *iptsd,
		struct ipts_stylus_data data)
{
	struct iptsd_stylus_device *stylus = iptsd->devices.active_stylus;

	float x = (float)data.x / IPTS_MAX_X;
	float y = (float)data.y / IPTS_MAX_Y;

	x = x * iptsd->config.width;
	y = y * iptsd->config.height;

	cone_set_tip(stylus->cone, x, y);
}

static int iptsd_stylus_handle_data(struct iptsd_context *iptsd,
		struct ipts_stylus_data data)
{
	int tx = 0;
	int ty = 0;

	struct iptsd_stylus_device *stylus = iptsd->devices.active_stylus;

	int prox = (data.mode & IPTS_STYLUS_REPORT_MODE_PROX) >> 0;
	int touch = (data.mode & IPTS_STYLUS_REPORT_MODE_TOUCH) >> 1;
	int button = (data.mode & IPTS_STYLUS_REPORT_MODE_BUTTON) >> 2;
	int rubber = (data.mode & IPTS_STYLUS_REPORT_MODE_RUBBER) >> 3;

	int btn_pen = prox * (1 - rubber);
	int btn_rubber = prox * rubber;

	if (prox) {
		iptsd_stylus_update_cone(iptsd, data);
		iptsd_stylus_tilt(data.altitude, data.azimuth, &tx, &ty);
	}

	iptsd_devices_emit(stylus->dev, EV_KEY, BTN_TOUCH, touch);
	iptsd_devices_emit(stylus->dev, EV_KEY, BTN_TOOL_PEN, btn_pen);
	iptsd_devices_emit(stylus->dev, EV_KEY, BTN_TOOL_RUBBER, btn_rubber);
	iptsd_devices_emit(stylus->dev, EV_KEY, BTN_STYLUS, button);

	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_X, data.x);
	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_Y, data.y);
	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_PRESSURE, data.pressure);
	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_MISC, data.timestamp);

	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_TILT_X, tx);
	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_TILT_Y, ty);

	int ret = iptsd_devices_emit(stylus->dev, EV_SYN, SYN_REPORT, 0);
	if (ret < 0)
		iptsd_err(ret, "Failed to emit stylus report");

	return ret;
}

static int iptsd_stylus_handle_serial_change(struct iptsd_context *iptsd,
		uint32_t serial)
{
	if (iptsd->devices.active_stylus->serial == serial)
		return 0;

	for (int i = 0; i < IPTSD_MAX_STYLI; i++) {
		if (iptsd->devices.styli[i].serial != serial)
			continue;

		iptsd->devices.active_stylus = &iptsd->devices.styli[i];
		return 0;
	}

	/*
	 * Before touching the screen for the first time, the stylus
	 * will report its serial as 0. Once you touch the screen,
	 * the serial will be reported correctly until you restart
	 * the machine.
	 */
	if (iptsd->devices.active_stylus->serial == 0) {
		iptsd->devices.active_stylus->serial = serial;
		return 0;
	}

	int ret = iptsd_devices_add_stylus(&iptsd->devices, serial);
	if (ret < 0)
		iptsd_err(ret, "Failed to add new stylus");

	return ret;
}

static int iptsd_stylus_read_tilt(struct iptsd_context *iptsd)
{
	struct ipts_stylus_data data;

	/*
	 * There can be more than one element, but if we send all of them
	 * the lines drawn by the stylus become very jagged. Our current
	 * theory is that the different elements together form an average
	 * value, that is then sent. For now, sending just the first
	 * one works well enough and doesnt produce jagged lines.
	 */
	int ret = iptsd_reader_read(&iptsd->reader, &data,
			sizeof(struct ipts_stylus_data));
	if (ret < 0) {
		iptsd_err(ret, "Received invalid data");
		return 0;
	}

	return iptsd_stylus_handle_data(iptsd, data);
}

static int iptsd_stylus_read_no_tilt(struct iptsd_context *iptsd)
{
	struct ipts_stylus_data full_data;
	struct ipts_stylus_data_no_tilt data;

	/*
	 * There can be more than one element, but if we send all of them
	 * the lines drawn by the stylus become very jagged. Our current
	 * theory is that the different elements together form an average
	 * value, that is then sent. For now, sending just the first
	 * one works well enough and doesnt produce jagged lines.
	 */
	int ret = iptsd_reader_read(&iptsd->reader, &data,
			sizeof(struct ipts_stylus_data_no_tilt));
	if (ret < 0) {
		iptsd_err(ret, "Received invalid data");
		return 0;
	}

	full_data.mode = data.mode;
	full_data.x = data.x;
	full_data.y = data.y;
	full_data.pressure = data.pressure * 4;
	full_data.altitude = 0;
	full_data.azimuth = 0;
	full_data.timestamp = 0;

	return iptsd_stylus_handle_data(iptsd, full_data);
}

static int iptsd_stylus_handle(struct iptsd_context *iptsd)
{
	struct ipts_stylus_report sreport;

	int ret = iptsd_reader_read(&iptsd->reader, &sreport,
			sizeof(struct ipts_stylus_report));
	if (ret < 0) {
		iptsd_err(ret, "Received invalid data");
		return 0;
	}

	ret = iptsd_stylus_read_tilt(iptsd);
	if (ret < 0)
		iptsd_err(ret, "Failed to handle stylus report");

	return ret;
}

static int iptsd_stylus_handle_serial(struct iptsd_context *iptsd, bool tilt)
{
	struct ipts_stylus_report_serial sreport;

	int ret = iptsd_reader_read(&iptsd->reader, &sreport,
			sizeof(struct ipts_stylus_report_serial));
	if (ret < 0) {
		iptsd_err(ret, "Received invalid data");
		return 0;
	}

	ret = iptsd_stylus_handle_serial_change(iptsd, sreport.serial);
	if (ret < 0) {
		iptsd_err(ret, "Failed to change stylus");
		return ret;
	}

	if (tilt)
		ret = iptsd_stylus_read_tilt(iptsd);
	else
		ret = iptsd_stylus_read_no_tilt(iptsd);

	if (ret < 0)
		iptsd_err(ret, "Failed to handle stylus report");

	return ret;
}

int iptsd_stylus_handle_input(struct iptsd_context *iptsd,
		struct ipts_payload_frame frame)
{
	uint32_t size = 0;

	while (size < frame.size) {
		struct ipts_report report;

		int ret = iptsd_reader_read(&iptsd->reader, &report,
				sizeof(struct ipts_report));
		if (ret < 0) {
			iptsd_err(ret, "Received invalid data");
			return 0;
		}

		switch (report.type) {
		case IPTS_REPORT_TYPE_STYLUS_NO_TILT:
			ret = iptsd_stylus_handle_serial(iptsd, false);
			break;
		case IPTS_REPORT_TYPE_STYLUS_TILT:
			ret = iptsd_stylus_handle(iptsd);
			break;
		case IPTS_REPORT_TYPE_STYLUS_TILT_SERIAL:
			ret = iptsd_stylus_handle_serial(iptsd, true);
			break;
		default:
			iptsd_reader_skip(&iptsd->reader, report.size);
			break;
		}

		if (ret < 0) {
			iptsd_err(ret, "Failed to handle stylus input");
			return ret;
		}

		size += report.size + sizeof(struct ipts_report);
	}

	return 0;
}
