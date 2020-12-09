// SPDX-License-Identifier: GPL-2.0-or-later

#include <dirent.h>
#include <ini.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>

#include <configure.h>

#include "config.h"
#include "ipts.h"
#include "constants.h"
#include "utils.h"

struct iptsd_config_device {
	int vendor;
	int product;
};

static bool iptsd_config_to_bool(const char *value)
{
	return strcasecmp(value, "true") == 0 ||
		strcasecmp(value, "yes") == 0 ||
		strcasecmp(value, "on") == 0 ||
		strcasecmp(value, "1") == 0;
}

static int iptsd_config_handler_device(void *user, const char *section,
		const char *name, const char *value)
{
	struct iptsd_config_device *dev = (struct iptsd_config_device *)user;

	if (strcmp(section, "Device") != 0)
		return 1;

	if (strcmp(name, "Vendor") == 0)
		dev->vendor = strtol(value, NULL, 16);

	if (strcmp(name, "Product") == 0)
		dev->product = strtol(value, NULL, 16);

	return 1;
}

static int iptsd_config_handler_conf(void *user, const char *section,
		const char *name, const char *value)
{
	struct iptsd_config *config = (struct iptsd_config *)user;

	if (strcmp(section, "Config") != 0)
		return 1;

	if (strcmp(name, "InvertX") == 0)
		config->invert_x = iptsd_config_to_bool(value);

	if (strcmp(name, "InvertY") == 0)
		config->invert_y = iptsd_config_to_bool(value);

	if (strcmp(name, "Width") == 0)
		config->width = strtol(value, NULL, 10);

	if (strcmp(name, "Height") == 0)
		config->height = strtol(value, NULL, 10);

	if (strcmp(name, "BlockOnPalm") == 0)
		config->block_on_palm = iptsd_config_to_bool(value);

	if (strcmp(name, "TouchThreshold") == 0)
		config->touch_threshold = strtol(value, NULL, 10);

	if (strcmp(name, "StabilityThreshold") == 0)
		config->stability_threshold = strtof(value, NULL);

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

	closedir(dir);
}

void iptsd_config_load(struct iptsd_config *config,
		struct ipts_device_info info)
{
	memset(config, 0, sizeof(struct iptsd_config));

	config->touch_threshold = CONTACT_TOUCH_THRESHOLD;
	config->stability_threshold = CONTACT_STABILITY_THRESHOLD;

	iptsd_config_load_dir(config, info, IPTSD_CONFIG_DIR);
	iptsd_config_load_dir(config, info, "./config");

	ini_parse(IPTSD_CONFIG_FILE, iptsd_config_handler_conf, config);
}

