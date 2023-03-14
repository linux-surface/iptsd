// SPDX-License-Identifier: GPL-2.0-or-later

#include "config.hpp"

#include "contacts/detection/algorithms/neutral.hpp"

#include <common/types.hpp>
#include <contacts/finder.hpp>
#include <ipts/protocol.hpp>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <configure.h>
#include <filesystem>
#include <ini.h>
#include <spdlog/spdlog.h>
#include <string>

namespace filesystem = std::filesystem;

namespace iptsd::config {

struct iptsd_config_device {
	u16 vendor;
	u16 product;
};

static bool to_bool(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	return value == "true" || value == "yes" || value == "on" || value == "1";
}

static int parse_dev(void *user, const char *c_section, const char *c_name, const char *c_value)
{
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	auto *dev = reinterpret_cast<struct iptsd_config_device *>(user);

	const std::string section(c_section);
	const std::string name(c_name);
	const std::string value(c_value);

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
	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
	auto *config = reinterpret_cast<Config *>(user);

	const std::string section(c_section);
	const std::string name(c_name);
	std::string value(c_value);

	if (section == "Config" && name == "InvertX")
		config->invert_x = to_bool(value);

	if (section == "Config" && name == "InvertY")
		config->invert_y = to_bool(value);

	if (section == "Config" && name == "Width")
		config->width = std::stof(value);

	if (section == "Config" && name == "Height")
		config->height = std::stof(value);

	if (section == "Touch" && name == "Disable")
		config->touch_disable = to_bool(value);

	if (section == "Touch" && name == "CheckCone")
		config->touch_check_cone = to_bool(value);

	if (section == "Touch" && name == "CheckStability")
		config->touch_check_stability = to_bool(value);

	if (section == "Touch" && name == "DisableOnPalm")
		config->touch_disable_on_palm = to_bool(value);

	if (section == "Touch" && name == "DisableOnStylus")
		config->touch_disable_on_stylus = to_bool(value);

	if (section == "Contacts" && name == "Detection") {
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		config->contacts_detection = value;
	}

	if (section == "Contacts" && name == "Neutral") {
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		config->contacts_neutral = value;
	}

	if (section == "Contacts" && name == "NeutralValue")
		config->contacts_neutral_value = std::stof(value);

	if (section == "Contacts" && name == "ActivationThreshold")
		config->contacts_activation_threshold = std::stof(value);

	if (section == "Contacts" && name == "DeactivationThreshold")
		config->contacts_deactivation_threshold = std::stof(value);

	if (section == "Contacts" && name == "TemporalWindow")
		config->contacts_temporal_window = std::stoi(value);

	if (section == "Contacts" && name == "SizeMin")
		config->contacts_size_min = std::stof(value);

	if (section == "Contacts" && name == "SizeMax")
		config->contacts_size_max = std::stof(value);

	if (section == "Contacts" && name == "AspectMin")
		config->contacts_aspect_min = std::stof(value);

	if (section == "Contacts" && name == "AspectMax")
		config->contacts_aspect_max = std::stof(value);

	if (section == "Contacts" && name == "SizeThreshold")
		config->contacts_size_thresh = std::stof(value);

	if (section == "Contacts" && name == "PositionThresholdMin")
		config->contacts_position_thresh_min = std::stof(value);

	if (section == "Contacts" && name == "PositionThresholdMax")
		config->contacts_position_thresh_max = std::stof(value);

	if (section == "Contacts" && name == "DistanceThreshold")
		config->contacts_distance_thresh = std::stof(value);

	if (section == "Stylus" && name == "Disable")
		config->stylus_disable = to_bool(value);

	if (section == "Cone" && name == "Angle")
		config->cone_angle = std::stof(value);

	if (section == "Cone" && name == "Distance")
		config->cone_distance = std::stof(value);

	if (section == "DFT" && name == "PositionMinAmp")
		config->dft_position_min_amp = std::stoi(value);

	if (section == "DFT" && name == "PositionMinMag")
		config->dft_position_min_mag = std::stoi(value);

	if (section == "DFT" && name == "PositionExp")
		config->dft_position_exp = std::stof(value);

	if (section == "DFT" && name == "ButtonMinMag")
		config->dft_button_min_mag = std::stoi(value);

	if (section == "DFT" && name == "FreqMinMag")
		config->dft_freq_min_mag = std::stoi(value);

	if (section == "DFT" && name == "TiltMinMag")
		config->dft_tilt_min_mag = std::stoi(value);

	if (section == "DFT" && name == "TiltDistance")
		config->dft_tilt_distance = std::stof(value);

	if (section == "DFT" && name == "TipDistance")
		config->dft_tip_distance = std::stof(value);

	return 1;
}

void Config::load_dir(const std::string &name, bool check_device)
{
	if (!filesystem::exists(name))
		return;

	for (const filesystem::directory_entry &p : filesystem::directory_iterator(name)) {
		if (!p.is_regular_file())
			continue;

		if (check_device) {
			struct iptsd_config_device dev {};
			ini_parse(p.path().c_str(), parse_dev, &dev);

			if (dev.vendor != this->vendor || dev.product != this->product)
				continue;
		}

		ini_parse(p.path().c_str(), parse_conf, this);
	}
}

Config::Config(i16 vendor, i16 product, std::optional<const ipts::Metadata> metadata)
	: vendor {vendor},
	  product {product}
{
	if (metadata.has_value()) {
		this->width = gsl::narrow<f32>(metadata->size.width) / 1e3f;
		this->height = gsl::narrow<f32>(metadata->size.height) / 1e3f;
		this->invert_x = metadata->transform.xx < 0;
		this->invert_y = metadata->transform.yy < 0;
	}

	this->load_dir(IPTSD_PRESET_DIR, true);
	this->load_dir("./etc/presets", true);

	if (const char *config_file_path = std::getenv("IPTSD_CONFIG_FILE")) {
		// Load configuration file from custom location
		// Mainly for developers to debug their work
		// without touching their known working main system configuration
		if (!filesystem::exists(config_file_path))
			throw std::runtime_error("IPTSD_CONFIG_FILE not found");

		if (ini_parse(config_file_path, parse_conf, this))
			throw std::runtime_error("IPTSD_CONFIG_FILE is corrupt");
	} else {
		if (filesystem::exists(IPTSD_CONFIG_FILE))
			ini_parse(IPTSD_CONFIG_FILE, parse_conf, this);

		this->load_dir(IPTSD_CONFIG_DIR, false);
	}
}

contacts::Config<f32> Config::contacts() const
{
	contacts::Config<f32> config {};

	config.detection.normalize = true;
	config.detection.activation_threshold = this->contacts_activation_threshold / 255;
	config.detection.deactivation_threshold = this->contacts_deactivation_threshold / 255;

	using Algorithm = contacts::detection::neutral::Algorithm;

	if (this->contacts_neutral == "mode")
		config.detection.neutral_value_algorithm = Algorithm::MODE;
	else if (this->contacts_neutral == "average")
		config.detection.neutral_value_algorithm = Algorithm::AVERAGE;
	else if (this->contacts_neutral == "constant")
		config.detection.neutral_value_algorithm = Algorithm::CONSTANT;

	config.detection.neutral_value_offset = this->contacts_neutral_value / 255;
	config.detection.neutral_value_backoff = 16; // TODO: config option

	const f32 diagonal = std::hypot(this->width, this->height);

	config.validation.track_validity = true;
	config.validation.size_limits = Vector2<f32> {
		this->contacts_size_min / diagonal,
		this->contacts_size_max / diagonal,
	};
	config.validation.aspect_limits = Vector2<f32> {
		this->contacts_aspect_min,
		this->contacts_aspect_max,
	};

	config.stability.check_temporal_stability = true;
	config.stability.temporal_window = this->contacts_temporal_window;
	config.stability.size_difference_threshold = this->contacts_size_thresh / diagonal;
	config.stability.distance_threshold = this->contacts_distance_thresh / diagonal;
	config.stability.movement_limits = Vector2<f32> {
		this->contacts_position_thresh_min / diagonal,
		this->contacts_position_thresh_max / diagonal,
	};

	return config;
}

} // namespace iptsd::config
