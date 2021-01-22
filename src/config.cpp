// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.hpp"

#include "constants.hpp"
#include "ipts.h"

#include <algorithm>
#include <cctype>
#include <configure.h>
#include <cstdint>
#include <filesystem>
#include <ini.h>
#include <string>

struct iptsd_config_device {
	uint16_t vendor;
	uint16_t product;
};

static bool to_bool(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	return value == "true" || value == "yes" || value == "on" || value == "1";
}

static int parse_dev(void *user, const char *c_section, const char *c_name, const char *c_value)
{
	struct iptsd_config_device *dev = (struct iptsd_config_device *)user;

	std::string section(c_section);
	std::string name(c_name);
	std::string value(c_value);

	if (section != "Device")
		return 1;

	if (name == "Vendor")
		dev->vendor = std::stoi(value, nullptr, 16);

	if (name == "Product")
		dev->product = std::stoi(value, nullptr, 16);

	return 1;
}

static int parse_conf(void *user, const char *c_section, const char *c_name, const char *c_value)
{
	IptsdConfig *config = (IptsdConfig *)user;

	std::string section(c_section);
	std::string name(c_name);
	std::string value(c_value);

	if (section != "Config")
		return 1;

	if (name == "InvertX")
		config->invert_x = to_bool(value);

	if (name == "InvertY")
		config->invert_y = to_bool(value);

	if (name == "Width")
		config->width = std::stoi(value);

	if (name == "Height")
		config->height = std::stoi(value);

	if (name == "BlockOnPalm")
		config->block_on_palm = to_bool(value);

	if (name == "TouchThreshold")
		config->touch_threshold = std::stoi(value);

	if (name == "StabilityThreshold")
		config->stability_threshold = std::stof(value);

	return 1;
}

void IptsdConfig::load_dir(std::string name, struct ipts_device_info info)
{
	if (!std::filesystem::exists(name))
		return;

	for (auto &p : std::filesystem::directory_iterator(name)) {
		if (!p.is_regular_file())
			continue;

		struct iptsd_config_device dev;
		ini_parse(p.path().c_str(), parse_dev, &dev);

		if (dev.vendor != info.vendor || dev.product != info.product)
			continue;

		ini_parse(p.path().c_str(), parse_conf, this);
	}
}

IptsdConfig::IptsdConfig(struct ipts_device_info info)
{
	this->touch_threshold = CONTACT_TOUCH_THRESHOLD;
	this->stability_threshold = CONTACT_STABILITY_THRESHOLD;

	this->load_dir(IPTSD_CONFIG_DIR, info);
	this->load_dir("./config", info);

	if (std::filesystem::exists(IPTSD_CONFIG_FILE))
		ini_parse(IPTSD_CONFIG_FILE, parse_conf, this);
}
