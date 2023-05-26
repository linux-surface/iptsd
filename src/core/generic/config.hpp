// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_GENERIC_CONFIG_HPP
#define IPTSD_CORE_GENERIC_CONFIG_HPP

#include <common/types.hpp>
#include <contacts/config.hpp>
#include <ipts/parser.hpp>

#include <optional>
#include <string>
#include <type_traits>

namespace iptsd::core {

class Config {
public:
	// [Config]
	bool invert_x = false;
	bool invert_y = false;

	f64 width = 0;
	f64 height = 0;

	// [Touch]
	bool touch_disable = false;
	bool touch_check_cone = true;
	bool touch_disable_on_palm = false;
	bool touch_disable_on_stylus = false;

	// [Contacts]
	std::string contacts_neutral = "mode";
	f64 contacts_neutral_value = 0;
	f64 contacts_activation_threshold = 24;
	f64 contacts_deactivation_threshold = 20;
	usize contacts_temporal_window = 3;
	f64 contacts_size_thresh_min = 0.1;
	f64 contacts_size_thresh_max = 0.5;
	f64 contacts_position_thresh_min = 0.1;
	f64 contacts_position_thresh_max = 2;
	f64 contacts_size_min = 0.2;
	f64 contacts_size_max = 2;
	f64 contacts_aspect_min = 1;
	f64 contacts_aspect_max = 2.5;

	// [Stylus]
	bool stylus_disable = false;
	f64 stylus_tip_distance = 0;

	// [Cone]
	f64 cone_angle = 30;
	f64 cone_distance = 5;

	// [DFT]
	usize dft_position_min_amp = 50;
	usize dft_position_min_mag = 2000;
	f64 dft_position_exp = -0.7;
	usize dft_button_min_mag = 1000;
	usize dft_freq_min_mag = 10000;
	usize dft_tilt_min_mag = 10000;
	f64 dft_tilt_distance = 0.6;

public:
	/*!
	 * Generates a configuration object for the contact detection library.
	 *
	 * @return A config object for contact detection using doubles.
	 */
	[[nodiscard]] contacts::Config<f64> contacts() const
	{
		contacts::Config<f64> config {};

		const f64 athresh = this->contacts_activation_threshold;
		const f64 dthresh = this->contacts_deactivation_threshold;

		config.detection.normalize = true;
		config.detection.activation_threshold = athresh / 255.0;
		config.detection.deactivation_threshold = dthresh / 255.0;

		using Algorithm = contacts::detection::neutral::Algorithm;

		if (this->contacts_neutral == "mode")
			config.detection.neutral_value_algorithm = Algorithm::MODE;
		else if (this->contacts_neutral == "average")
			config.detection.neutral_value_algorithm = Algorithm::AVERAGE;
		else if (this->contacts_neutral == "constant")
			config.detection.neutral_value_algorithm = Algorithm::CONSTANT;

		const f64 nval_offset = this->contacts_neutral_value;

		config.detection.neutral_value_offset = nval_offset / 255.0;
		config.detection.neutral_value_backoff = 16; // TODO: config option

		const f64 diagonal = std::hypot(this->width, this->height);

		config.validation.track_validity = true;
		config.validation.size_limits = Vector2<f64> {
			this->contacts_size_min / diagonal,
			this->contacts_size_max / diagonal,
		};
		config.validation.aspect_limits = Vector2<f64> {
			this->contacts_aspect_min,
			this->contacts_aspect_max,
		};

		config.stability.check_temporal_stability = true;
		config.stability.temporal_window = this->contacts_temporal_window;
		config.stability.size_threshold = Vector2<f64> {
			this->contacts_size_thresh_min / diagonal,
			this->contacts_size_thresh_max / diagonal,
		};
		config.stability.position_threshold = Vector2<f64> {
			this->contacts_position_thresh_min / diagonal,
			this->contacts_position_thresh_max / diagonal,
		};

		return config;
	}
};

} // namespace iptsd::core

#endif // IPTSD_CORE_GENERIC_CONFIG_HPP
