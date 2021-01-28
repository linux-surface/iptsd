// SPDX-License-Identifier: GPL-2.0-or-later

#include "touch.hpp"

#include "config.hpp"
#include "context.hpp"
#include "control.hpp"
#include "devices.hpp"
#include "heatmap.hpp"
#include "ipts.h"
#include "protocol.h"
#include "reader.hpp"
#include "touch-processing.hpp"

#include <cstddef>
#include <cstdint>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <memory>
#include <vector>

static void lift_mt(TouchDevice *dev)
{
	dev->emit(EV_ABS, ABS_MT_TRACKING_ID, -1);
}

static void emit_mt(TouchDevice *dev, TouchInput in)
{
	dev->emit(EV_ABS, ABS_MT_TRACKING_ID, in.index);
	dev->emit(EV_ABS, ABS_MT_POSITION_X, in.x);
	dev->emit(EV_ABS, ABS_MT_POSITION_Y, in.y);

	dev->emit(EV_ABS, ABS_MT_TOOL_TYPE, MT_TOOL_FINGER);
	dev->emit(EV_ABS, ABS_MT_TOOL_X, in.x);
	dev->emit(EV_ABS, ABS_MT_TOOL_Y, in.y);

	dev->emit(EV_ABS, ABS_MT_ORIENTATION, in.orientation);
	dev->emit(EV_ABS, ABS_MT_TOUCH_MAJOR, in.major);
	dev->emit(EV_ABS, ABS_MT_TOUCH_MINOR, in.minor);
}

static void lift_st(TouchDevice *dev)
{
	dev->emit(EV_KEY, BTN_TOUCH, 0);
}

static void emit_st(TouchDevice *dev, TouchInput in)
{
	dev->emit(EV_KEY, BTN_TOUCH, 1);
	dev->emit(EV_ABS, ABS_X, in.x);
	dev->emit(EV_ABS, ABS_Y, in.y);
}

static void handle_single(TouchDevice *touch, size_t max, bool blocked)
{
	for (size_t i = 0; i < max; i++) {
		TouchInput in = touch->processor.inputs[i];

		if (in.index != -1 && !in.is_stable)
			return;

		if (in.index == -1 || in.is_palm || blocked)
			continue;

		emit_st(touch, in);
		return;
	}

	lift_st(touch);
}

static void handle_multi(TouchDevice *touch, size_t max, bool blocked)
{
	for (size_t i = 0; i < max; i++) {
		TouchInput in = touch->processor.inputs[i];

		touch->emit(EV_ABS, ABS_MT_SLOT, in.slot);

		if (in.index != -1 && !in.is_stable)
			continue;

		if (in.index == -1 || in.is_palm || blocked) {
			lift_mt(touch);
			continue;
		}

		emit_mt(touch, in);
	}
}

static void handle_heatmap(IptsdContext *iptsd, Heatmap *hm)
{
	bool blocked = false;

	TouchDevice *touch = &iptsd->devices->touch;
	uint8_t max_contacts = iptsd->control->info.max_contacts;

	touch->processor.process(hm);

	if (iptsd->config->block_on_palm) {
		for (uint8_t i = 0; i < max_contacts; i++)
			blocked = blocked || touch->processor.inputs[i].is_palm;
	}

	handle_multi(touch, max_contacts, blocked);
	handle_single(touch, max_contacts, blocked);

	touch->emit(EV_SYN, SYN_REPORT, 0);
}

Heatmap *get_heatmap(IptsdContext *iptsd)
{
	auto dim = iptsd->reader->read<struct ipts_heatmap_dim>();
	return iptsd->devices->touch.get_heatmap(dim.width, dim.height);
}

void iptsd_touch_handle_input(IptsdContext *iptsd, struct ipts_payload_frame frame)
{
	uint32_t size = 0;
	Heatmap *hm = nullptr;

	while (size < frame.size) {
		auto report = iptsd->reader->read<struct ipts_report>();

		switch (report.type) {
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP_DIM:
			hm = get_heatmap(iptsd);
			break;
		case IPTS_REPORT_TYPE_TOUCH_HEATMAP:
			if (!hm)
				break;

			iptsd->reader->read(hm->data.data(), std::size(hm->data));
			break;
		default:
			iptsd->reader->skip(report.size);
			break;
		}

		size += report.size + sizeof(struct ipts_report);
	}

	if (!hm)
		return;

	handle_heatmap(iptsd, hm);
}
