// SPDX-License-Identifier: GPL-2.0-or-later

#include <ipts/control.hpp>
#include <ipts/ipts.h>
#include <ipts/protocol.h>

#include <CLI/CLI.hpp>
#include <fmt/format.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

static void print_buffer(char *buffer, size_t size)
{
	for (size_t i = 0; i < size; i += 32) {
		for (size_t j = 0; j < 32; j++) {
			if (i + j >= size)
				continue;

			fmt::print("{:02X} ", static_cast<unsigned char>(buffer[i + j]));
		}

		fmt::print("\n");
	}

	fmt::print("\n");
}

int main(int argc, char *argv[])
{
	auto filename = std::filesystem::path{};

	auto app = CLI::App { "Read raw IPTS data" };
	app.add_option("-b,--binary", filename, "Write data to binary file instead of stdout")
		->type_name("FILE");

	CLI11_PARSE(app, argc, argv);

	auto file = std::ofstream {};
	if (!filename.empty()) {
		file.exceptions(std::ios::badbit | std::ios::failbit);
		file.open(filename, std::ios::out | std::ios::binary);
	}

	IptsControl ctrl;

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

		ctrl.read(data, ctrl.info.buffer_size);
		struct ipts_data *header = (struct ipts_data *)data;

		if (file) {
			file.write(data, sizeof(struct ipts_data) + header->size);
		} else {
			auto const header_type = header->type;
			auto const header_buffer = header->buffer;
			auto const header_size = header->size;

			fmt::print("====== Buffer: {} == Type: {} == Size: {} =====\n",
				   header_type, header_buffer, header_size);

			print_buffer(&data[sizeof(struct ipts_data)], header->size);
		}

		ctrl.send_feedback();
	}

	return 0;
}
