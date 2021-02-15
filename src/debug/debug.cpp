// SPDX-License-Identifier: GPL-2.0-or-later

#include <ipts/control.hpp>
#include <ipts/ipts.h>
#include <ipts/protocol.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
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

			std::printf("%02hhX ", buffer[i + j]);
		}

		std::printf("\n");
	}

	std::printf("\n");
}

static void help(std::string pname)
{
	std::cout << pname << " - Read raw IPTS data" << std::endl;
	std::cout << std::endl;
	std::cout << "Usage:" << std::endl;
	std::cout << "  " << pname << std::endl;
	std::cout << "  " << pname << " --binary <file>" << std::endl;
	std::cout << "  " << pname << " --help" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h | --help                  Show this help text" << std::endl;
	std::cout << "  -b <file> | --binary <file>  Write data to binary file instead of stdout"
		  << std::endl;
}

int main(int argc, char *argv[])
{
	std::ofstream file;
	std::vector<std::string> args(argv, &argv[argc]);

	for (size_t i = 1; i < std::size(args); i++) {
		std::string arg = args[i];

		if (arg == "-h" || arg == "--help") {
			help(args[0]);
			return 0;
		}

		if (arg == "-b" || arg == "--binary") {
			if (i + 1 >= std::size(args)) {
				std::cerr << "Error: Missing command line argument to " << arg
					  << std::endl
					  << std::endl;
				help(args[0]);
				return EXIT_FAILURE;
			}

			std::string filename = args[++i];
			file = std::ofstream(filename, std::ios::out | std::ios::binary);
			if (!file) {
				std::cerr << "Failed to open file '" << filename << "'"
					  << std::endl;
				return EXIT_FAILURE;
			}
		} else {
			std::cerr << "Error: Unknown command line argument " << arg << std::endl
				  << std::endl;
			help(args[0]);
			return EXIT_FAILURE;
		}
	}

	IptsdControl ctrl;

	std::printf("Vendor:       %04X\n", ctrl.info.vendor);
	std::printf("Product:      %04X\n", ctrl.info.product);
	std::printf("Version:      %u\n", ctrl.info.version);
	std::printf("Buffer Size:  %u\n", ctrl.info.buffer_size);
	std::printf("Max Contacts: %d\n", ctrl.info.max_contacts);
	std::printf("\n");

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
			std::cout << "====== Buffer: " << header->buffer
				  << " == Type: " << header->type << " == Size: " << header->size
				  << " =====" << std::endl;
			print_buffer(&data[sizeof(struct ipts_data)], header->size);
		}

		ctrl.send_feedback();
	}

	return 0;
}
