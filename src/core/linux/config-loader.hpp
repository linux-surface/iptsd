// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_CONFIG_LOADER_HPP
#define IPTSD_CORE_LINUX_CONFIG_LOADER_HPP

#include "configure.h"

#include <common/casts.hpp>
#include <common/types.hpp>
#include <core/generic/config.hpp>
#include <core/generic/device.hpp>

#include <INIReader.h>
#include <fmt/format.h>

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace iptsd::core::linux {

class ConfigLoader {
private:
	Config m_config {};
	DeviceInfo m_info;

public:
	ConfigLoader(const DeviceInfo &info, std::optional<const ipts::Metadata> metadata)
		: m_info {info}
	{
		namespace filesystem = std::filesystem;

		if (metadata.has_value()) {
			m_config.width = casts::to<f64>(metadata->size.width) / 1e3;
			m_config.height = casts::to<f64>(metadata->size.height) / 1e3;
			m_config.invert_x = metadata->transform.xx < 0;
			m_config.invert_y = metadata->transform.yy < 0;
		}

		this->load_dir(IPTSD_PRESET_DIR, true);
		this->load_dir("./etc/presets", true);

		/*
		 * Load configuration file from custom location.
		 *
		 * Mainly for developers to debug their work without touching their
		 * known working main system configuration.
		 */
		if (const char *config_file_path = std::getenv("IPTSD_CONFIG_FILE")) {
			this->load_file(config_file_path);
			return;
		}

		if (filesystem::exists(IPTSD_CONFIG_FILE))
			this->load_file(IPTSD_CONFIG_FILE);

		this->load_dir(IPTSD_CONFIG_DIR, false);
	}

	/*!
	 * The loaded config object.
	 *
	 * @return The configuration data that was loaded for the given device.
	 */
	[[nodiscard]] Config config() const
	{
		return m_config;
	}

private:
	/*!
	 * Load all configuration files from a directory.
	 *
	 * @param[in] path The path to the directory.
	 * @param[in] check_device If true, check if the config is meant for the current device.
	 */
	void load_dir(const std::string &path, const bool check_device)
	{
		namespace filesystem = std::filesystem;

		if (!filesystem::exists(path))
			return;

		for (const filesystem::directory_entry &p : filesystem::directory_iterator(path)) {
			if (!p.is_regular_file())
				continue;

			if (check_device) {
				u16 vendor = 0;
				u16 product = 0;

				this->load_device(p.path(), vendor, product);

				// Ignore this file if it is meant for a different device.
				if (m_info.vendor != vendor || m_info.product != product)
					continue;
			}

			this->load_file(p.path());
		}
	}

	/*!
	 * Determines for which device a config file is meant.
	 *
	 * @param[in] path The path to the config file.
	 * @param[out] vendor The vendor ID the config is targeting.
	 * @param[out] vendor The product ID the config is targeting.
	 */
	void load_device(const std::string &path, u16 &vendor, u16 &product)
	{
		const INIReader ini {path};

		if (ini.ParseError() != 0)
			throw std::runtime_error(fmt::format("Failed to parse {}", path));

		vendor = 0;
		product = 0;

		this->get(ini, "Device", "Vendor", vendor);
		this->get(ini, "Device", "Product", product);
	}

	/*!
	 * Loads configuration data from a single file.
	 *
	 * @param[in] path The file to load and parse.
	 */
	void load_file(const std::string &path)
	{
		const INIReader ini {path};

		if (ini.ParseError() != 0)
			throw std::runtime_error(fmt::format("Failed to parse {}", path));

		// clang-format off

		this->get(ini, "Config", "InvertX", m_config.invert_x);
		this->get(ini, "Config", "InvertY", m_config.invert_y);
		this->get(ini, "Config", "Width", m_config.width);
		this->get(ini, "Config", "Height", m_config.height);

		this->get(ini, "Touch", "Disable", m_config.touch_disable);
		this->get(ini, "Touch", "DisableOnPalm", m_config.touch_disable_on_palm);
		this->get(ini, "Touch", "DisableOnStylus", m_config.touch_disable_on_stylus);
		this->get(ini, "Touch", "Overshoot", m_config.touch_overshoot);

		this->get(ini, "Contacts", "Neutral", m_config.contacts_neutral);
		this->get(ini, "Contacts", "NeutralValue", m_config.contacts_neutral_value);
		this->get(ini, "Contacts", "ActivationThreshold", m_config.contacts_activation_threshold);
		this->get(ini, "Contacts", "DeactivationThreshold", m_config.contacts_deactivation_threshold);
		this->get(ini, "Contacts", "SizeThresholdMin", m_config.contacts_size_thresh_min);
		this->get(ini, "Contacts", "SizeThresholdMax", m_config.contacts_size_thresh_max);
		this->get(ini, "Contacts", "PositionThresholdMin", m_config.contacts_position_thresh_min);
		this->get(ini, "Contacts", "PositionThresholdMax", m_config.contacts_position_thresh_max);
		this->get(ini, "Contacts", "OrientationThresholdMin", m_config.contacts_orientation_thresh_min);
		this->get(ini, "Contacts", "OrientationThresholdMax", m_config.contacts_orientation_thresh_max);
		this->get(ini, "Contacts", "SizeMin", m_config.contacts_size_min);
		this->get(ini, "Contacts", "SizeMax", m_config.contacts_size_max);
		this->get(ini, "Contacts", "AspectMin", m_config.contacts_aspect_max);
		this->get(ini, "Contacts", "AspectMax", m_config.contacts_aspect_max);

		this->get(ini, "Stylus", "Disable", m_config.stylus_disable);
		this->get(ini, "Stylus", "TipDistance", m_config.stylus_tip_distance);

		this->get(ini, "DFT", "PositionMinAmp", m_config.dft_position_min_amp);
		this->get(ini, "DFT", "PositionMinMag", m_config.dft_position_min_mag);
		this->get(ini, "DFT", "PositionExp", m_config.dft_position_exp);
		this->get(ini, "DFT", "ButtonMinMag", m_config.dft_button_min_mag);
		this->get(ini, "DFT", "FreqMinMag", m_config.dft_freq_min_mag);
		this->get(ini, "DFT", "TiltMinMag", m_config.dft_tilt_min_mag);
		this->get(ini, "DFT", "TiltDistance", m_config.dft_tilt_distance);

		// Legacy options that are kept for compatibility
		this->get(ini, "DFT", "TipDistance", m_config.stylus_tip_distance);
		this->get(ini, "Contacts", "SizeThreshold", m_config.contacts_size_thresh_max);

		// clang-format on
	}

	/*!
	 * Loads a value from a config file.
	 *
	 * @param[in] ini The loaded file.
	 * @param[in] section The section where the option is found.
	 * @param[in] name The name of the config option.
	 * @param[in,out] value The default value as well as the destination of the new value.
	 */
	template <class T>
	void get(const INIReader &ini,
		 const std::string &section,
		 const std::string &name,
		 T &value) const
	{
		if constexpr (std::is_same_v<T, bool>)
			value = ini.GetBoolean(section, name, value);
		else if constexpr (std::is_integral_v<T>)
			value = casts::to<T>(ini.GetInteger(section, name, casts::to<long>(value)));
		else if constexpr (std::is_floating_point_v<T>)
			value = gsl::narrow_cast<T>(ini.GetReal(section, name, value));
		else if constexpr (std::is_same_v<T, std::string>)
			value = ini.GetString(section, name, value);
		else
			throw std::runtime_error("Loading this type is not implemented!");
	}
};

} // namespace iptsd::core::linux

#endif // IPTSD_CORE_LINUX_CONFIG_LOADER_HPP
