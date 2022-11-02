// SPDX-License-Identifier: GPL-2.0-or-later

#include <common/signal.hpp>
#include <hid/device.hpp>
#include <ipts/device.hpp>
#include <ipts/protocol.hpp>

#include <CLI/CLI.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <gsl/gsl>
#include <iostream>
#include <iterator>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

struct PrettyBuf {
	const u8 *data;
	size_t size;
};

template <> struct fmt::formatter<gsl::span<const u8>> {
	char hexfmt = 'x';
	char prefix = 'n';

	constexpr auto parse(format_parse_context &ctx)
	{
		auto it = ctx.begin(), end = ctx.end();

		while (it != end && *it != '}') {
			if (*it == 'x' || *it == 'X') {
				hexfmt = *it++;
			} else if (*it == 'n' || *it == 'o' || *it == 'O') {
				prefix = *it++;
			} else {
				throw format_error("invalid format");
			}
		}

		return it;
	}

	template <class FormatContext>
	auto format(const gsl::span<const u8> &buf, FormatContext &ctx)
	{
		char const *pfxstr = prefix == 'o' ? "{:04x}: " : "{:04X}: ";
		char const *fmtstr = hexfmt == 'x' ? "{:02x} " : "{:02X} ";

		auto it = ctx.out();
		for (size_t i = 0; i < buf.size(); i += 32) {
			size_t j = 0;

			if (prefix != 'n')
				it = format_to(it, pfxstr, i);

			for (; j < 8 && i + j < buf.size(); j++)
				it = format_to(it, fmtstr, buf[i + j]);

			it = format_to(it, " ");

			for (; j < 16 && i + j < buf.size(); j++)
				it = format_to(it, fmtstr, buf[i + j]);

			it = format_to(it, " ");

			for (; j < 24 && i + j < buf.size(); j++)
				it = format_to(it, fmtstr, buf[i + j]);

			it = format_to(it, " ");

			for (; j < 32 && i + j < buf.size(); j++)
				it = format_to(it, fmtstr, buf[i + j]);

			it = format_to(it, "\n");
		}

		return format_to(it, "\n");
	}
};

namespace iptsd::debug::dump {

struct iptsd_dump_header {
	i16 vendor;
	i16 product;
	std::size_t buffer_size;
};

static int main(gsl::span<char *> args)
{
	CLI::App app {};
	std::filesystem::path path;
	std::filesystem::path filename;

	app.add_option("DEVICE", path, "The hidraw device to read from.")
		->type_name("FILE")
		->required();

	app.add_option("OUTPUT", filename, "Output binary data to this file in addition to stdout.")
		->type_name("FILE");

	CLI11_PARSE(app, args.size(), args.data());

	std::atomic_bool should_exit = false;

	const auto _sigterm = common::signal<SIGTERM>([&](int) { should_exit = true; });
	const auto _sigint = common::signal<SIGINT>([&](int) { should_exit = true; });

	std::ofstream file;
	if (!filename.empty()) {
		file.exceptions(std::ios::badbit | std::ios::failbit);
		file.open(filename, std::ios::out | std::ios::binary);
	}

	ipts::Device dev {path};

	// Get the buffer size from the HID descriptor
	std::size_t buffer_size = dev.buffer_size();
	std::vector<u8> buffer(buffer_size);

	if (file) {
		struct iptsd_dump_header header {};
		header.vendor = dev.vendor();
		header.product = dev.product();
		header.buffer_size = buffer_size;

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		file.write(reinterpret_cast<char *>(&header), sizeof(header));
	}

	spdlog::info("Vendor:       {:04X}", dev.vendor());
	spdlog::info("Product:      {:04X}", dev.product());
	spdlog::info("Buffer Size:  {}", buffer_size);

	// Count errors, if we receive 50 continuous errors, chances are pretty good that
	// something is broken beyond repair and the program should exit.
	i32 errors = 0;

	// Enable multitouch mode
	dev.set_mode(true);

	while (!should_exit) {
		if (errors >= 50) {
			spdlog::error("Encountered 50 continuous errors, aborting...");
			break;
		}

		try {
			ssize_t size = dev.read(buffer);

			if (!dev.is_touch_data(buffer[0]))
				continue;

			if (file) {
				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				file.write(reinterpret_cast<char *>(&size), sizeof(size));

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				file.write(reinterpret_cast<char *>(buffer.data()),
					   gsl::narrow<std::streamsize>(buffer_size));
			}

			const gsl::span<const u8> buf(buffer.data(), size);

			spdlog::info("== Size: {} ==", size);
			spdlog::info("{:ox}", buf);
		} catch (std::exception &e) {
			spdlog::warn(e.what());
			errors++;
			continue;
		}

		// Reset error count
		errors = 0;
	}

	// Disable multitouch mode
	dev.set_mode(false);

	return 0;
}

} // namespace iptsd::debug::dump

int main(int argc, char *argv[])
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::debug::dump::main(gsl::span<char *>(argv, argc));
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
