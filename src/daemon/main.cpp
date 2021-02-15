// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.hpp"
#include "context.hpp"
#include "data.hpp"
#include "devices.hpp"
#include "reader.hpp"

#include <common/types.hpp>
#include <common/utils.hpp>
#include <ipts/control.hpp>
#include <ipts/ipts.h>

#include <chrono>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <thread>

using namespace std::chrono;

bool should_reset;

static void signal_reset(int sig)
{
	should_reset = true;
}

static bool iptsd_loop(IptsdContext *iptsd)
{
	u32 doorbell = iptsd->control->doorbell();
	u32 diff = doorbell - iptsd->control->current_doorbell;
	u32 size = iptsd->control->info.buffer_size;

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

	system_clock::time_point timeout = system_clock::now() + 5s;
	std::printf("Connected to device %04X:%04X\n", info.vendor, info.product);

	iptsd.config = new IptsdConfig(info);
	iptsd.reader = new IptsdReader(info.buffer_size);
	iptsd.devices = new DeviceManager(info, iptsd.config);

	while (true) {
		if (iptsd_loop(&iptsd))
			timeout = system_clock::now() + 5s;

		std::this_thread::sleep_for(timeout > system_clock::now() ? 10ms : 200ms);

		if (should_reset) {
			iptsd.control->reset();
			should_reset = false;
		}
	}

	return 0;
}
