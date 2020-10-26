// SPDX-License-Identifer: GPL-2.0-or-later

#include <dirent.h>
#include <ini.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "config.h"
#include "ipts.h"
#include "syscall.h"

struct iptsd_config_device {
	int vendor;
	int product;
};

static bool iptsd_config_bool(const char *value)
{
	if (!strcmp(value, "true"))
		return true;

	if (!strcmp(value, "True"))
		return true;

	if (!strcmp(value, "1"))
		return true;

	return false;
}

static int iptsd_config_handler_device(void *user, const char *section,
		const char *name, const char *value)
{
	struct iptsd_config_device *dev = (struct iptsd_config_device *)user;

	if (strcmp(section, "Device"))
		return 1;

	if (!strcmp(name, "Vendor"))
		dev->vendor = strtol(value, NULL, 16);

	if (!strcmp(name, "Product"))
		dev->product = strtol(value, NULL, 16);

	return 1;
}

static int iptsd_config_handler_conf(void *user, const char *section,
		const char *name, const char *value)
{
	struct iptsd_config *config = (struct iptsd_config *)user;

	if (strcmp(section, "Config"))
		return 1;

	if (!strcmp(name, "InvertX"))
		config->invert_x = iptsd_config_bool(value);

	if (!strcmp(name, "InvertY"))
		config->invert_y = iptsd_config_bool(value);

	if (!strcmp(name, "Width"))
		config->width = strtol(value, NULL, 10);

	if (!strcmp(name, "Height"))
		config->height = strtol(value, NULL, 10);

	if (!strcmp(name, "BlockOnPalm"))
		config->block_on_palm = iptsd_config_bool(value);

	return 1;
}

static void iptsd_config_load_dir(struct iptsd_config *config,
		struct ipts_device_info info, const char *name)
{
	char path[PATH_MAX];
	struct dirent *entry;
	DIR *dir = opendir(name);

	if (!dir)
		return;

	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type != DT_REG)
			continue;

		struct iptsd_config_device dev;

		snprintf(path, PATH_MAX, "%s/%s", name, entry->d_name);
		ini_parse(path, iptsd_config_handler_device, &dev);

		if (dev.vendor != info.vendor || dev.product != info.product)
			continue;

		ini_parse(path, iptsd_config_handler_conf, config);
	}
}

void iptsd_config_load(struct iptsd_config *config,
		struct ipts_device_info info)
{
	iptsd_config_load_dir(config, info, "/usr/share/ipts");
	iptsd_config_load_dir(config, info, "/usr/local/share/ipts");
	iptsd_config_load_dir(config, info, "./config");

	ini_parse("/etc/ipts.conf", iptsd_config_handler_conf, config);
}

