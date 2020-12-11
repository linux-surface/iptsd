// SPDX-License-Identifier: GPL-2.0-or-later

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"

int iptsd_reader_read(struct iptsd_reader *reader, void *dest, size_t size)
{
	if (!reader || !dest)
		return -EFAULT;

	if (reader->current + size > reader->size)
		return -EFAULT;

	memcpy(dest, &reader->data[reader->current], size);
	reader->current += size;
	return 0;
}

void iptsd_reader_skip(struct iptsd_reader *reader, size_t size)
{
	if (!reader)
		return;

	reader->current += size;
}

int iptsd_reader_init(struct iptsd_reader *reader, size_t size)
{
	if (!reader)
		return -EFAULT;

	reader->data = calloc(size, sizeof(char));
	if (!reader->data)
		return -ENOMEM;

	reader->size = size;
	reader->current = 0;
	return 0;
}

void iptsd_reader_reset(struct iptsd_reader *reader)
{
	if (!reader)
		return;

	reader->current = 0;
	memset(reader->data, 0, reader->size);
}

void iptsd_reader_free(struct iptsd_reader *reader)
{
	if (reader && reader->data)
		free(reader->data);
}
