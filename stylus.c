// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/uinput.h>
#include <stdint.h>
#include <stdio.h>

#include "context.h"
#include "devices.h"
#include "protocol.h"
#include "stylus-processing.h"
#include "utils.h"

static int iptsd_stylus_handle_data(struct iptsd_context *iptsd,
		struct ipts_stylus_data data)
{
	int sx, sy;
	int tx, ty;

	struct iptsd_stylus_device *stylus = iptsd->devices.active_stylus;

	int prox = (data.mode & IPTS_STYLUS_REPORT_MODE_PROX) >> 0;
	int touch = (data.mode & IPTS_STYLUS_REPORT_MODE_TOUCH) >> 1;
	int button = (data.mode & IPTS_STYLUS_REPORT_MODE_BUTTON) >> 2;
	int rubber = (data.mode & IPTS_STYLUS_REPORT_MODE_RUBBER) >> 3;

	int btn_pen = prox * (1 - rubber);
	int btn_rubber = prox * rubber;

	iptsd_stylus_processing_smooth(&stylus->processor,
			data.x, data.y, &sx, &sy);

	iptsd_stylus_processing_tilt(&stylus->processor,
			data.altitude, data.azimuth, &tx, &ty);

	if (prox == 0)
		iptsd_stylus_processing_flush(&stylus->processor);

	iptsd_devices_emit(stylus->dev, EV_KEY, BTN_TOUCH, touch);
	iptsd_devices_emit(stylus->dev, EV_KEY, BTN_TOOL_PEN, btn_pen);
	iptsd_devices_emit(stylus->dev, EV_KEY, BTN_TOOL_RUBBER, btn_rubber);
	iptsd_devices_emit(stylus->dev, EV_KEY, BTN_STYLUS, button);

	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_X, sx);
	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_Y, sy);
	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_PRESSURE, data.pressure);
	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_MISC, data.timestamp);

	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_TILT_X, tx);
	iptsd_devices_emit(stylus->dev, EV_ABS, ABS_TILT_Y, ty);

	int ret = iptsd_devices_emit(stylus->dev, EV_SYN, SYN_REPORT, 0);
	if (ret < 0)
		iptsd_err(ret, "Failed to emit stylus report");

	return ret;
}

static int iptsd_stylus_change_serial(struct iptsd_context *iptsd,
		uint32_t serial)
{
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

static int iptsd_stylus_handle_tilt_serial(struct iptsd_context *iptsd,
		struct ipts_report *report)
{
	int pos = 0;
	struct ipts_stylus_report_serial *sreport =
		(struct ipts_stylus_report_serial *)report->data;

	if (iptsd->devices.active_stylus->serial != sreport->serial) {
		int ret = iptsd_stylus_change_serial(iptsd, sreport->serial);
		if (ret < 0) {
			iptsd_err(ret, "Failed to change stylus");
			return ret;
		}
	}

	for (int i = 0; i < sreport->elements; i++) {
		struct ipts_stylus_data *data =
			(struct ipts_stylus_data *)&sreport->data[pos];

		int ret = iptsd_stylus_handle_data(iptsd, *data);
		if (ret < 0) {
			iptsd_err(ret, "Failed to handle stylus report");
			return ret;
		}
	}

	return 0;
}

static int iptsd_stylus_handle_tilt(struct iptsd_context *iptsd,
		struct ipts_report *report)
{
	int pos = 0;
	struct ipts_stylus_report *sreport =
		(struct ipts_stylus_report *)report->data;

	for (int i = 0; i < sreport->elements; i++) {
		struct ipts_stylus_data *data =
			(struct ipts_stylus_data *)&sreport->data[pos];

		int ret = iptsd_stylus_handle_data(iptsd, *data);
		if (ret < 0) {
			iptsd_err(ret, "Failed to handle stylus report");
			return ret;
		}
	}

	return 0;
}

static int iptsd_stylus_handle_no_tilt(struct iptsd_context *iptsd,
		struct ipts_report *report)
{
	int pos = 0;
	struct ipts_stylus_data full_data;

	struct ipts_stylus_report_serial *sreport =
		(struct ipts_stylus_report_serial *)report->data;

	if (iptsd->devices.active_stylus->serial != sreport->serial) {
		int ret = iptsd_stylus_change_serial(iptsd, sreport->serial);
		if (ret < 0) {
			iptsd_err(ret, "Failed to change stylus");
			return ret;
		}
	}

	for (int i = 0; i < sreport->elements; i++) {
		struct ipts_stylus_data_no_tilt *data =
			(struct ipts_stylus_data_no_tilt *)&sreport->data[pos];

		full_data.mode = data->mode;
		full_data.x = data->x;
		full_data.y = data->y;
		full_data.pressure = data->pressure * 4;
		full_data.altitude = 0;
		full_data.azimuth = 0;
		full_data.timestamp = 0;

		int ret = iptsd_stylus_handle_data(iptsd, full_data);
		if (ret < 0) {
			iptsd_err(ret, "Failed to handle stylus report");
			return ret;
		}
	}

	return 0;
}

int iptsd_stylus_handle_input(struct iptsd_context *iptsd,
		struct ipts_payload_frame *frame)
{
	int pos = 0;

	while (pos < frame->size) {
		int ret = 0;
		struct ipts_report *report =
			(struct ipts_report *)&frame->data[pos];

		switch (report->type) {
		case IPTS_REPORT_TYPE_STYLUS_NO_TILT:
			ret = iptsd_stylus_handle_no_tilt(iptsd, report);
			break;
		case IPTS_REPORT_TYPE_STYLUS_TILT:
			ret = iptsd_stylus_handle_tilt(iptsd, report);
			break;
		case IPTS_REPORT_TYPE_STYLUS_TILT_SERIAL:
			ret = iptsd_stylus_handle_tilt_serial(iptsd, report);
			break;
		}

		if (ret < 0) {
			iptsd_err(ret, "Failed to handle stylus input");
			return ret;
		}

		pos += report->size + sizeof(struct ipts_report);
	}

	return 0;
}
