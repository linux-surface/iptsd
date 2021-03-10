// SPDX-License-Identifier: GPL-2.0-or-later

#include <ipts/control.hpp>
#include <ipts/ipts.h>
#include <ipts/protocol.h>

#include <CLI/CLI.hpp>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <span>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

struct PrettyBuf {
	char const *data;
	size_t size;
};

template <> struct fmt::formatter<PrettyBuf> {
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

	template <class FormatContext> auto format(PrettyBuf const &buf, FormatContext &ctx)
	{
		char const *pfxstr = prefix == 'o' ? "{:04x}: " : "{:04X}: ";
		char const *fmtstr = hexfmt == 'x' ? "{:02x} " : "{:02X} ";

		auto it = ctx.out();
		for (size_t i = 0; i < buf.size; i += 32) {
			size_t j = 0;

			if (prefix != 'n') {
				it = format_to(it, pfxstr, i);
			}

			for (; j < 8 && i + j < buf.size; j++) {
				it = format_to(it, fmtstr,
					       static_cast<unsigned char>(buf.data[i + j]));
			}

			it = format_to(it, " ");

			for (; j < 16 && i + j < buf.size; j++) {
				it = format_to(it, fmtstr,
					       static_cast<unsigned char>(buf.data[i + j]));
			}

			it = format_to(it, " ");

			for (; j < 24 && i + j < buf.size; j++) {
				it = format_to(it, fmtstr,
					       static_cast<unsigned char>(buf.data[i + j]));
			}

			it = format_to(it, " ");

			for (; j < 32 && i + j < buf.size; j++) {
				it = format_to(it, fmtstr,
					       static_cast<unsigned char>(buf.data[i + j]));
			}

			it = format_to(it, "\n");
		}

		return format_to(it, "\n");
	}
};

namespace iptsd::debug::dbg {

static int main(int argc, char *argv[])
{
	auto filename = std::filesystem::path {};

	auto app = CLI::App {"Read raw IPTS data"};
	app.add_option("-b,--binary", filename, "Write data to binary file instead of stdout")
		->type_name("FILE");

	CLI11_PARSE(app, argc, argv);

	auto file = std::ofstream {};
	if (!filename.empty()) {
		file.exceptions(std::ios::badbit | std::ios::failbit);
		file.open(filename, std::ios::out | std::ios::binary);
	}

	ipts::Control ctrl;

	fmt::print("Vendor:       {:04X}\n", ctrl.info.vendor);
	fmt::print("Product:      {:04X}\n", ctrl.info.product);
	fmt::print("Version:      {}\n", ctrl.info.version);
	fmt::print("Buffer Size:  {}\n", ctrl.info.buffer_size);
	fmt::print("Max Contacts: {}\n", ctrl.info.max_contacts);
	fmt::print("\n");

	char *data = new char[ctrl.info.buffer_size];

	while (true) {
		uint32_t doorbell = ctrl.doorbell();
		if (doorbell <= ctrl.current_doorbell)
			continue;

		ctrl.read(std::span<u8>(reinterpret_cast<u8 *>(data), ctrl.info.buffer_size));
		struct ipts_data *header = (struct ipts_data *)data;

		if (file) {
			file.write(data, sizeof(struct ipts_data) + header->size);
		} else {
			auto const header_type = header->type;
			auto const header_buffer = header->buffer;
			auto const header_size = header->size;

			auto const buf = PrettyBuf {&data[sizeof(struct ipts_data)], header->size};

			fmt::print("====== Buffer: {} == Type: {} == Size: {} =====\n", header_type,
				   header_buffer, header_size);

			fmt::print("{:ox}\n", buf);
		}

		ctrl.send_feedback();
	}

	return 0;
}

} // namespace iptsd::debug::dbg

int main(int argc, char *argv[])
{
	spdlog::set_pattern("[%X.%e] [%^%l%$] %v");

	try {
		return iptsd::debug::dbg::main(argc, argv);
	} catch (std::exception &e) {
		spdlog::error(e.what());
		return EXIT_FAILURE;
	}
}
