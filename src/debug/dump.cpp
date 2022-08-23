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
	u8 const *data;
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

static int main(gsl::span<char *> args)
{
	std::filesystem::path device;
	std::filesystem::path filename;

	CLI::App app {"Read raw IPTS data"};
	app.add_option("-d,--device", device, "The hidraw device to open")
		->type_name("FILE")
		->required();
	app.add_option("-b,--binary", filename, "Write data to binary file instead of stdout")
		->type_name("FILE");

	CLI11_PARSE(app, args.size(), args.data());

	std::ofstream file;
	if (!filename.empty()) {
		file.exceptions(std::ios::badbit | std::ios::failbit);
		file.open(filename, std::ios::out | std::ios::binary);
	}

	ipts::Device dev(device);

	std::atomic_bool should_exit {false};

	auto const _sigterm = common::signal<SIGTERM>([&](int) { should_exit = true; });
	auto const _sigint = common::signal<SIGINT>([&](int) { should_exit = true; });

	// Get the buffer size from the HID descriptor
	std::size_t buffer_size = dev.buffer_size();
	std::vector<u8> buffer(buffer_size);

	if (file) {
		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
		file.write(reinterpret_cast<char *>(&buffer_size), sizeof(buffer_size));
	}

	fmt::print("Vendor:       {:04X}\n", dev.vendor());
	fmt::print("Product:      {:04X}\n", dev.product());
	fmt::print("Buffer Size:  {}\n", buffer_size);
	fmt::print("\n");

	// Enable multitouch mode
	dev.set_mode(true);

	while (!should_exit) {
		try {
			ssize_t rsize = dev.read(buffer);

			if (file) {
				if (!dev.is_touch_data(buffer[0]))
					continue;

				// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
				file.write(reinterpret_cast<char *>(buffer.data()),
					   gsl::narrow<std::streamsize>(buffer_size));

				continue;
			}

			const gsl::span<const u8> buf {buffer.data(), (std::size_t)rsize};
			fmt::print("== Size: {} ==\n", rsize);
			fmt::print("{:ox}\n", buf);
		} catch (std::exception &e) {
			spdlog::error(e.what());
			break;
		}
	}

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
