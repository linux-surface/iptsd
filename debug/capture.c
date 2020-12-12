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

struct iptsd_control ctrl;
FILE *file = NULL;

static void __exit(int error)
{
	int ret = iptsd_control_stop(&ctrl);
	if (ret < 0)
		iptsd_err(ret, "Failed to stop IPTS");

	if (file)
		fclose(file);

	exit(error);
}

static void print_help(const char *pname)
{
	printf("%s - Dump IPTS data to binary file\n", pname);
	printf("\n");
	printf("Usage:\n");
	printf("  %s <file>\n", pname);
	printf("\n");
	printf("Options:\n");
	printf("  -h | --help    Show this help text\n");
}

int main(int argc, char **argv)
{
	signal(SIGINT, __exit);
	signal(SIGTERM, __exit);
	memset(&ctrl, 0, sizeof(struct iptsd_control));

	if (argc != 2) {
		printf("Invalid command line arguments:\n");
		printf("  See '%s --help' for more information\n", argv[0]);
		return 1;
	}

	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		print_help(argv[0]);
		return 1;
	}

	int ret = iptsd_control_start(&ctrl);
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

	file = fopen(argv[1], "wb");

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

		size_t len = sizeof(struct ipts_data) + ((struct ipts_data *)data)->size;
		fwrite(data, sizeof(char), len, file);

		ret = iptsd_control_send_feedback(&ctrl);
		if (ret < 0) {
			iptsd_err(ret, "Failed to send feedback");
			__exit(ret);
		}
	}

	return 0;
}
