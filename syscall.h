/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_SYSCALL_H_
#define _IPTSD_SYSCALL_H_

int iptsd_syscall_open(const char *file, int flags);
int iptsd_syscall_close(int fd);
int iptsd_syscall_read(int fd, void *buf, size_t count);
int iptsd_syscall_write(int fd, void *buf, size_t count);
int iptsd_syscall_ioctl(int fd, unsigned long request, void *data);
char *iptsd_syscall_strerr(int err);

#endif /* _IPTSD_SYSCALL_H_ */

