// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

int iptsd_syscall_open(const char *file, int flags)
{
	int fd = open(file, flags);
	if (fd != -1)
		return fd;

	return -errno;
}

int iptsd_syscall_close(int fd)
{
	int ret = close(fd);
	if (ret != -1)
		return ret;

	return -errno;
}

int iptsd_syscall_read(int fd, void *buf, size_t count)
{
	int ret = read(fd, buf, count);
	if (ret != -1)
		return ret;

	return -errno;
}

int iptsd_syscall_write(int fd, void *buf, size_t count)
{
	int ret = write(fd, buf, count);
	if (ret != -1)
		return ret;

	return -errno;
}

int iptsd_syscall_ioctl(int fd, unsigned long request, void *data)
{
	int ret = ioctl(fd, request, data);
	if (ret != -1)
		return ret;

	return -errno;
}

void iptsd_syscall_err(int err, const char *file,
		int line, const char *format, ...)
{
	va_list args;
	va_start(args, format);

	fprintf(stderr, "ERROR: %s:%d: ", file, line);
	vfprintf(stderr, format, args);
	fprintf(stderr, ": %s\n", strerror(-err));

	va_end(args);
}

