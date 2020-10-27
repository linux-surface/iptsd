// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "control.h"
#include "ipts.h"
#include "syscall.h"

int iptsd_control_current_file(struct iptsd_control *control)
{
	return control->files[control->current_doorbell % IPTS_BUFFERS];
}

int iptsd_control_ready(struct iptsd_control *control)
{
	uint8_t ready;

	int fd = iptsd_control_current_file(control);
	int ret = iptsd_syscall_ioctl(fd,
			IPTS_IOCTL_GET_DEVICE_READY, &ready);

	if (ret < 0) {
		iptsd_err(ret, "Failed get ready status");
		return ret;
	}

	return ready;
}

void iptsd_control_wait_for_device(struct iptsd_control *control)
{
	for (int i = 0; i < 5; i++) {
		if (iptsd_control_ready(control) > 0)
			break;

		sleep(1);
	}
}

static int __iptsd_control_send_feedback(struct iptsd_control *control,
		int file)
{
	iptsd_control_wait_for_device(control);

	int ret = iptsd_syscall_ioctl(file, IPTS_IOCTL_SEND_FEEDBACK, NULL);
	if (ret < 0)
		iptsd_err(ret, "Failed to send feedback");

	return ret;
}

int iptsd_control_send_feedback(struct iptsd_control *control)
{
	int file = iptsd_control_current_file(control);
	int ret = __iptsd_control_send_feedback(control, file);

	if (ret >= 0)
		control->current_doorbell++;

	return ret;
}

int iptsd_control_flush(struct iptsd_control *control)
{
	for (int i = 0; i < IPTS_BUFFERS; i++) {
		int file = control->files[i];
		int ret = __iptsd_control_send_feedback(control, file);

		if (ret < 0) {
			iptsd_err(ret, "Failed to flush buffers");
			return ret;
		}
	}

	return 0;
}

uint32_t iptsd_control_doorbell(struct iptsd_control *control)
{
	iptsd_control_wait_for_device(control);

	uint32_t doorbell;

	int fd = iptsd_control_current_file(control);
	int ret = iptsd_syscall_ioctl(fd, IPTS_IOCTL_GET_DOORBELL, &doorbell);

	if (ret < 0) {
		iptsd_err(ret, "Failed to get doorbell");
		return ret;
	}

	/*
	 * If the new doorbell is lower than the value we have stored,
	 * the device has been reset below our feet (i.e. through suspending).
	 *
	 * We send feedback to clear all buffers and reset the stored value.
	 */
	if (control->current_doorbell > doorbell) {
		ret = iptsd_control_flush(control);
		if (ret < 0)
			return ret;

		control->current_doorbell = doorbell;
	}

	return doorbell;
}

int iptsd_control_device_info(struct iptsd_control *control)
{
	iptsd_control_wait_for_device(control);

	int fd = iptsd_control_current_file(control);
	int ret = iptsd_syscall_ioctl(fd, IPTS_IOCTL_GET_DEVICE_INFO,
			&control->device_info);

	if (ret < 0)
		iptsd_err(ret, "Failed to get device info");

	return ret;
}

int iptsd_control_start(struct iptsd_control *control)
{
	char name[32];

	for (int i = 0; i < IPTS_BUFFERS; i++) {
		snprintf(name, sizeof(name), "/dev/ipts/%d", i);

		int ret = iptsd_syscall_open(name, O_RDONLY);
		if (ret >= 0) {
			control->files[i] = ret;
			continue;
		}

		iptsd_err(ret, "Failed to open %s", name);
		return ret;
	}

	int ret = iptsd_control_flush(control);
	if (ret < 0)
		return ret;

	ret = iptsd_control_device_info(control);
	if (ret < 0)
		return ret;

	ret = iptsd_control_doorbell(control);
	if (ret < 0)
		return ret;

	control->current_doorbell = ret;

	return 0;
}

int iptsd_control_read(struct iptsd_control *control, void *buf, size_t count)
{
	iptsd_control_wait_for_device(control);

	int fd = iptsd_control_current_file(control);
	int ret = iptsd_syscall_read(fd, buf, count);

	if (ret < 0)
		iptsd_err(ret, "Failed to read from buffer");

	return ret;
}

int iptsd_control_stop(struct iptsd_control *control)
{
	for (int i = 0; i < IPTS_BUFFERS; i++) {
		int ret = iptsd_syscall_close(control->files[i]);
		if (ret >= 0)
			continue;

		iptsd_err(ret, "Failed to close file %d", i);
	}

	return 0;
}

