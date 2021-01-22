// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.hpp"
#include "context.hpp"
#include "control.hpp"
#include "data.hpp"
#include "devices.hpp"
#include "ipts.h"
#include "reader.hpp"
#include "utils.hpp"

#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <iostream>

using namespace std::chrono;

bool should_reset;

static void signal_reset(int sig)
{
	should_reset = true;
}

static bool iptsd_loop(IptsdContext *iptsd)
{
	uint32_t doorbell = iptsd->control->doorbell();
	uint32_t diff = doorbell - iptsd->control->current_doorbell;
	uint32_t size = iptsd->control->info.buffer_size;

	while (doorbell > iptsd->control->current_doorbell) {
		iptsd->control->read(iptsd->reader->data, size);

		try {
			iptsd_data_handle_input(iptsd);
		} catch (IptsdReaderException &e) {
			std::cerr << e.what() << std::endl;
		}

		iptsd->control->send_feedback();
		iptsd->reader->reset();
	}

	return diff > 0;
}

int main(void)
{
	IptsdContext iptsd;

	should_reset = false;
	Utils::signal(SIGUSR1, signal_reset);

	iptsd.control = new IptsdControl();
	struct ipts_device_info info = iptsd.control->info;

	system_clock::time_point timeout = system_clock::now() + seconds(5);
	std::printf("Connected to device %04X:%04X\n", info.vendor, info.product);

	iptsd.config = new IptsdConfig(info);
	iptsd.reader = new IptsdReader(info.buffer_size);
	iptsd.devices = new DeviceManager(info, iptsd.config);

	while (true) {
		if (iptsd_loop(&iptsd))
			timeout = system_clock::now() + seconds(5);

		if (timeout > system_clock::now())
			Utils::msleep(10);
		else
			Utils::msleep(200);

		if (should_reset) {
			iptsd.control->reset();
			should_reset = false;
		}
	}

	return 0;
}
