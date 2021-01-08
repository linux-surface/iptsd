// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "src/control.h"
#include "src/ipts.h"
#include "src/protocol.h"
#include "src/utils.h"

static struct iptsd_control ctrl;
static FILE *file = NULL;

static void __exit(int error)
{
	int ret = iptsd_control_stop(&ctrl);
	if (ret < 0)
		iptsd_err(ret, "Failed to stop IPTS");

	if (file)
		fclose(file);

	exit(error);
}

static void print_buffer(char *buffer, size_t size)
{
	for (size_t i = 0; i < size; i += 32) {
		for (size_t j = 0; j < 32; j++) {
			if (i + j >= size)
				continue;

			printf("%02hhX ", buffer[i + j]);
		}

		printf("\n");
	}

	printf("\n");
}

static int file_open(const char *path, const char *mode, FILE **out)
{
	FILE *f;

	f = fopen(path, mode);
	if (!f)
		return -errno;

	*out = f;
	return 0;
}

static int file_write(FILE *file, char *data, size_t len)
{
	size_t n = fwrite(data, sizeof(char), len, file);
	return n == len ? 0 : -EIO;
}

static void print_usage(const char *pname)
{
	printf("Usage:\n");
	printf("  %s\n", pname);
	printf("  %s --binary <file>\n", pname);
	printf("  %s --help\n", pname);
}

static void print_help(const char *pname)
{
	printf("%s - Read raw IPTS data\n", pname);
	printf("\n");
	printf("Usage:\n");
	printf("  %s\n", pname);
	printf("  %s --binary <file>\n", pname);
	printf("  %s --help\n", pname);
	printf("\n");
	printf("Options:\n");
	printf("  -h | --help                   Show this help text\n");
	printf("  -b <file> | --binary <file>   Write data to binary file instead of stdout\n");
}

int main(int argc, char **argv)
{
	const char *binfile = NULL;
	int ret = 0;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			print_help(argv[0]);
			return 0;
		} else if (!strcmp(argv[i], "-b") || !strcmp(argv[i], "--binary")) {
			if (i + 1 >= argc) {
				printf("Error: Missing command line argument to '%s'\n\n", argv[i]);
				print_usage(argv[0]);
				return 1;
			}

			binfile = argv[++i];
		} else {
			printf("Error: Unknown command line argument '%s'\n\n", argv[i]);
			print_usage(argv[0]);
			return 1;
		}
	}

	ret = iptsd_utils_signal(SIGINT, __exit);
	if (ret < 0) {
		iptsd_err(ret, "Failed to register signal handler");
		return ret;
	}

	ret = iptsd_utils_signal(SIGTERM, __exit);
	if (ret < 0) {
		iptsd_err(ret, "Failed to register signal handler");
		return ret;
	}

	memset(&ctrl, 0, sizeof(struct iptsd_control));

	if (binfile) {
		ret = file_open(binfile, "wb", &file);
		if (ret < 0) {
			iptsd_err(ret, "Failed to open file '%s'", binfile);
			return ret;
		}
	}

	ret = iptsd_control_start(&ctrl);
	if (ret < 0) {
		iptsd_err(ret, "Failed to start IPTS");
		return ret;
	}

	struct ipts_device_info info = ctrl.device_info;

	printf("Vendor:       %04X\n", info.vendor);
	printf("Product:      %04X\n", info.product);
	printf("Version:      %u\n", info.version);
	printf("Buffer Size:  %u\n", info.buffer_size);
	printf("Max Contacts: %d\n", info.max_contacts);
	printf("\n");

	char *data = calloc(info.buffer_size, sizeof(char));
	if (!data) {
		iptsd_err(-ENOMEM, "Failed to allocate data buffer");
		__exit(-ENOMEM);
	}

	while (true) {
		int64_t doorbell = iptsd_control_doorbell(&ctrl);
		if (doorbell < 0) {
			iptsd_err(doorbell, "Failed to get doorbell");
			__exit(doorbell);
		}

		if (doorbell <= ctrl.current_doorbell)
			continue;

		int ret = iptsd_control_read(&ctrl, data, info.buffer_size);
		if (ret < 0) {
			iptsd_err(ret, "Failed to read IPTS data");
			__exit(ret);
		}

		struct ipts_data *header = (struct ipts_data *)data;

		if (file) {
			ret = file_write(file, data, sizeof(struct ipts_data) + header->size);
			if (ret < 0) {
				iptsd_err(ret, "Failed to write data to file");
				__exit(ret);
			}
		} else {
			printf("====== Buffer: %d == Type: %d == Size: %d ======\n",
					header->buffer, header->type, header->size);
			print_buffer(&data[sizeof(struct ipts_data)], header->size);
		}

		ret = iptsd_control_send_feedback(&ctrl);
		if (ret < 0) {
			iptsd_err(ret, "Failed to send feedback");
			__exit(ret);
		}
	}

	return 0;
}
