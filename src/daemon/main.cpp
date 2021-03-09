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
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <thread>

using namespace std::chrono;

static bool iptsd_loop(IptsdContext &ctx)
{
	u32 doorbell = ctx.control.doorbell();
	u32 diff = doorbell - ctx.control.current_doorbell;

	while (doorbell > ctx.control.current_doorbell) {
		ctx.control.read(ctx.parser.buffer());

		try {
			ctx.parser.parse();
		} catch (std::out_of_range &e) {
			spdlog::error(e.what());
		}

		ctx.control.send_feedback();
	}

	return diff > 0;
}

static int iptsd_main()
{
	IptsdContext ctx {};

	std::atomic_bool should_exit {false};
	std::atomic_bool should_reset {false};

	auto const _sigusr1 = iptsd::common::signal<SIGUSR1>([&](int) { should_reset = true; });
	auto const _sigterm = iptsd::common::signal<SIGTERM>([&](int) { should_exit = true; });
	auto const _sigint = iptsd::common::signal<SIGINT>([&](int) { should_exit = true; });

	struct ipts_device_info info = ctx.control.info;
	system_clock::time_point timeout = system_clock::now() + 5s;

	spdlog::info("Connected to device {:04X}:{:04X}\n", info.vendor, info.product);

	ctx.parser.on_singletouch = [&](const auto &data) { iptsd_singletouch_input(ctx, data); };
	ctx.parser.on_stylus = [&](const auto &data) { iptsd_stylus_input(ctx, data); };
	ctx.parser.on_heatmap = [&](const auto &data) { iptsd_touch_input(ctx, data); };

	while (true) {
		if (iptsd_loop(ctx))
			timeout = system_clock::now() + 5s;

		std::this_thread::sleep_for(timeout > system_clock::now() ? 10ms : 200ms);

		if (should_reset) {
			spdlog::info("Resetting touch sensor");

			ctx.control.reset();
			should_reset = false;
		}

		if (should_exit) {
			spdlog::info("Stopping");

			return EXIT_FAILURE;
		}
	}

	return 0;
}

int main()
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd_main();
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
