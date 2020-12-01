/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_READER_H_
#define _IPTSD_READER_H_

#include <stddef.h>

struct iptsd_reader {
	char *data;
	size_t size;
	size_t current;
};

int iptsd_reader_read(struct iptsd_reader *reader, void *dest, size_t size);
void iptsd_reader_skip(struct iptsd_reader *reader, size_t size);
int iptsd_reader_init(struct iptsd_reader *reader, size_t size);
void iptsd_reader_reset(struct iptsd_reader *reader);
void iptsd_reader_free(struct iptsd_reader *reader);

#endif /* _IPTSD_READER_H_ */
