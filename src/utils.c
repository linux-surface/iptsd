// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "utils.h"

int iptsd_utils_open(const char *file, int flags)
{
	int fd = open(file, flags);
	if (fd != -1)
		return fd;

	return -errno;
}

int iptsd_utils_close(int fd)
{
	int ret = close(fd);
	if (ret != -1)
		return ret;

	return -errno;
}

int iptsd_utils_read(int fd, void *buf, size_t count)
{
	int ret = read(fd, buf, count);
	if (ret != -1)
		return ret;

	return -errno;
}

int iptsd_utils_write(int fd, void *buf, size_t count)
{
	int ret = write(fd, buf, count);
	if (ret != -1)
		return ret;

	return -errno;
}

int iptsd_utils_ioctl(int fd, unsigned long request, void *data)
{
	int ret = ioctl(fd, request, data);
	if (ret != -1)
		return ret;

	return -errno;
}

int iptsd_utils_signal(int signum, void (*handler)(int))
{
	struct sigaction sig;

	memset(&sig, 0, sizeof(struct sigaction));
	sig.sa_handler = handler;

	int ret = sigaction(signum, &sig, NULL);
	if (ret != -1)
		return ret;

	return -errno;
}

void iptsd_utils_err(int err, const char *file,
		int line, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	fprintf(stderr, "ERROR: %s:%d: ", file, line);
	vfprintf(stderr, format, args);
	fprintf(stderr, ": %s\n", strerror(-err));

	va_end(args);
}

int iptsd_utils_msec_timestamp(uint64_t *ts)
{
	struct timespec t;

	if (!ts)
		return -EINVAL;

	int ret = clock_gettime(CLOCK_MONOTONIC, &t);
	if (ret == -1)
		return -errno;

	*ts = (uint64_t)t.tv_sec * 1000 + t.tv_nsec / 1000000;
	return 0;
}

