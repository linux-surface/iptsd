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

static void __exit(int error)
{
	int ret = iptsd_control_stop(&ctrl);
	if (ret < 0)
		iptsd_err(ret, "Failed to stop IPTS");

	exit(error);
}

static void print_buffer(char *buffer, size_t size, size_t offset)
{
	for (size_t i = offset; i < size; i += 32) {
		for (size_t j = 0; j < 32; j++) {
			if (i + j >= size)
				continue;

			printf("%02hhX ", buffer[i + j]);
		}

		printf("\n");
	}

	printf("\n");
}

int main(void)
{
	signal(SIGINT, __exit);
	signal(SIGTERM, __exit);
	memset(&ctrl, 0, sizeof(struct iptsd_control));

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

		printf("====== Buffer: %d == Type: %d == Size: %d ======\n",
				header->buffer, header->type, header->size);

		print_buffer(data, header->size, sizeof(struct ipts_data));

		ret = iptsd_control_send_feedback(&ctrl);
		if (ret < 0) {
			iptsd_err(ret, "Failed to send feedback");
			__exit(ret);
		}
	}

	return 0;
}
