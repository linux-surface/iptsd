// SPDX-License-Identifier: GPL-2.0-or-later

#include "protocol.h"
#include "stylus-processing.h"

void iptsd_stylus_processing_add(struct iptsd_stylus_processor *proc,
		struct ipts_stylus_data data)
{
	if (!proc)
		return;

	proc->timestamp = data.timestamp;
	proc->mode = proc->mode | data.mode;

	proc->x += data.x;
	proc->y += data.y;
	proc->pressure += data.pressure;
	proc->altitude += data.altitude;
	proc->azimuth += data.azimuth;
	proc->count++;
}

void iptsd_stylus_processing_get(struct iptsd_stylus_processor *proc,
		struct ipts_stylus_data *out)
{
	if (!proc || !out)
		return;

	if (proc->count == 0)
		return;

	out->timestamp = proc->timestamp;
	out->mode = proc->mode;

	out->x = proc->x / proc->count;
	out->y = proc->y / proc->count;
	out->pressure = proc->pressure / proc->count;
	out->altitude = proc->altitude / proc->count;
	out->azimuth = proc->azimuth / proc->count;
}

void iptsd_stylus_processing_flush(struct iptsd_stylus_processor *proc)
{
	if (!proc)
		return;

	proc->timestamp = 0;
	proc->mode = 0;

	proc->x = 0;
	proc->y = 0;
	proc->pressure = 0;
	proc->altitude = 0;
	proc->azimuth = 0;
	proc->count = 0;
}
