// SPDX-License-Identifier: GPL-2.0-or-later

#include "context.hpp"
#include "devices.hpp"
#include "dft.hpp"
#include "stylus.hpp"
#include "touch.hpp"

#include <common/signal.hpp>
#include <common/types.hpp>
#include <config/config.hpp>
#include <ipts/device.hpp>
#include <ipts/parser.hpp>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <thread>

using namespace std::chrono;

namespace iptsd::daemon {

static int start()
{
	std::atomic_bool should_exit = false;

	auto const _sigterm = common::signal<SIGTERM>([&](int) { should_exit = true; });
	auto const _sigint = common::signal<SIGINT>([&](int) { should_exit = true; });

	ipts::Device device;

	auto &meta = device.meta_data;
	if (meta.has_value()) {
		auto &t = meta->transform;
		auto &u = meta->unknown2;

		spdlog::info("Metadata:");
		spdlog::info("rows={}, columns={}", meta->size.rows, meta->size.columns);
		spdlog::info("width={}, height={}", meta->size.width, meta->size.height);
		spdlog::info("transform=[{},{},{},{},{},{}]", t.xx, t.yx, t.tx, t.xy, t.yy, t.ty);
		spdlog::info("unknown={}, [{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}]",
			     meta->unknown1, u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7],
			     u[8], u[9], u[10], u[11], u[12], u[13], u[14], u[15]);
	}

	config::Config config {device.vendor_id, device.product_id, meta};

	// Check if a config was found
	if (config.width == 0 || config.height == 0)
		throw std::runtime_error("No display config for this device was found!");

	Context ctx {config, meta};
	spdlog::info("Connected to device {:04X}:{:04X}", device.vendor_id, device.product_id);

	ipts::Parser parser {};
    if (!config.stylus_disable) {
        parser.on_dft = [&](const auto &dft, auto &stylus) {
            device.process_end();
            
            iptsd_dft_input(ctx, dft, stylus);
        };
        parser.on_stylus = [&](const auto &data) {
            device.process_end();
            
            IPTSHIDReport report;
            memset(&report, 0, sizeof(IPTSHIDReport));
            iptsd_stylus_input(ctx, data, report);
            device.send_hid_report(report);
        };
    } else
        spdlog::warn("Stylus is disabled!");
    
    if (!config.touch_disable)
        parser.on_heatmap = [&](const auto &data) {
            device.process_end();
            
            if (ctx.devices.stylus->active && ctx.config.touch_disable_on_stylus)
                return;
            
            IPTSHIDReport report;
            memset(&report, 0, sizeof(IPTSHIDReport));
            if (iptsd_touch_input(ctx, data, report))
                device.send_hid_report(report);
            };
    else
        spdlog::warn("Touchscreen is disabled!");

	// Count errors, if we receive 10 continuous errors, chances are pretty good that
	// something is broken beyond repair and the program should exit.
	i32 errors = 0;
	while (!should_exit) {
		if (errors >= 10) {
			spdlog::error("Encountered 10 continuous errors, aborting...");
			break;
		}

		try {
            gsl::span<u8> buffer = device.read();
            
            device.process_begin();
			parser.parse(buffer);
            device.process_end();
		} catch (std::system_error &e) {
            if (device.should_reinit)
                device.reset();
            errors++;
            spdlog::warn(e.what());
            continue;
        } catch (std::exception &e) {
			spdlog::warn(e.what());
			errors++;

			// Sleep for 100ms to allow the device to get back to normal state
			std::this_thread::sleep_for(100ms);

			continue;
		}

		// Reset error count
		errors = 0;
	}

    spdlog::info("Stopping");
    device.process_end();
	// If iptsd was stopped from outside, return no error
	if (!should_exit)
		return EXIT_FAILURE;

	return 0;
}

} // namespace iptsd::daemon

int main(int argc, char **argv)
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::daemon::start();
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
