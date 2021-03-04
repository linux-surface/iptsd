// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.hpp"
#include "context.hpp"
#include "devices.hpp"
#include "singletouch.hpp"
#include "stylus.hpp"
#include "touch.hpp"

#include <common/signal.hpp>
#include <common/types.hpp>
#include <ipts/control.hpp>
#include <ipts/ipts.h>
#include <ipts/parser.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <thread>

using namespace std::chrono;

static bool iptsd_loop(IptsdContext *iptsd)
{
	u32 doorbell = iptsd->control->doorbell();
	u32 diff = doorbell - iptsd->control->current_doorbell;

	while (doorbell > iptsd->control->current_doorbell) {
		iptsd->control->read(iptsd->parser->buffer());

		try {
			iptsd->parser->parse();
		} catch (std::out_of_range &e) {
			std::cerr << e.what() << std::endl;
		}

		iptsd->control->send_feedback();
	}

	return diff > 0;
}

int main(void)
{
	IptsdContext iptsd;

	auto should_exit = std::atomic_bool {false};
	auto should_reset = std::atomic_bool {false};

	auto const _sigusr1 = iptsd::common::signal<SIGUSR1>([&](int) { should_reset = true; });
	auto const _sigterm = iptsd::common::signal<SIGTERM>([&](int) { should_exit = true; });
	auto const _sigint = iptsd::common::signal<SIGINT>([&](int) { should_exit = true; });

	iptsd.control = new IptsControl();
	struct ipts_device_info info = iptsd.control->info;

	system_clock::time_point timeout = system_clock::now() + 5s;
	std::printf("Connected to device %04X:%04X\n", info.vendor, info.product);

	iptsd.config = new IptsdConfig(info);
	iptsd.devices = new DeviceManager(iptsd.config);
	iptsd.parser = new IptsParser(info.buffer_size);

	iptsd.parser->on_singletouch = [&](auto data) { iptsd_singletouch_input(&iptsd, data); };
	iptsd.parser->on_stylus = [&](auto data) { iptsd_stylus_input(&iptsd, data); };
	iptsd.parser->on_heatmap = [&](auto data) { iptsd_touch_input(&iptsd, data); };

	while (true) {
		if (iptsd_loop(&iptsd))
			timeout = system_clock::now() + 5s;

		std::this_thread::sleep_for(timeout > system_clock::now() ? 10ms : 200ms);

		if (should_reset) {
			iptsd.control->reset();
			should_reset = false;
		}

		if (should_exit)
			return EXIT_FAILURE;
	}

	return 0;
}
