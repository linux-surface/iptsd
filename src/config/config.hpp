// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONFIG_CONFIG_HPP
#define IPTSD_CONFIG_CONFIG_HPP

#include <common/types.hpp>
#include <contacts/config.hpp>
#include <ipts/parser.hpp>

#include <optional>
#include <string>

namespace iptsd::config {

class Config {
public:
	// [Device]
	i16 vendor;
	i16 product;

	// [Config]
	bool invert_x = false;
	bool invert_y = false;

	f32 width = 0;
	f32 height = 0;

	// [Touch]
	bool touch_disable = false;
	bool touch_check_cone = true;
	bool touch_check_stability = true;
	bool touch_disable_on_palm = false;
	bool touch_disable_on_stylus = false;

	// [Contacts]
	std::string contacts_detection = "basic";
	std::string contacts_neutral = "mode";
	f32 contacts_neutral_value = 0;
	f32 contacts_activation_threshold = 24;
	f32 contacts_deactivation_threshold = 20;
	u32 contacts_temporal_window = 3;
	f32 contacts_size_min = 0.2;
	f32 contacts_size_max = 2;
	f32 contacts_aspect_min = 1;
	f32 contacts_aspect_max = 2.5;
	f32 contacts_size_thresh = 0.1;
	f32 contacts_position_thresh_min = 0.2;
	f32 contacts_position_thresh_max = 2;
	f32 contacts_distance_thresh = 1;

	// [Stylus]
	bool stylus_disable = false;

	// [Cone]
	f32 cone_angle = 30;
	f32 cone_distance = 5;

	// [DFT]
	u16 dft_position_min_amp = 50;
	u16 dft_position_min_mag = 2000;
	f32 dft_position_exp = -0.7;
	u16 dft_button_min_mag = 1000;
	u16 dft_freq_min_mag = 10000;
	u16 dft_tilt_min_mag = 10000;
	f32 dft_tilt_distance = 0.6;
	f32 dft_tip_distance = 0;

public:
	template <class T> [[nodiscard]] contacts::Config<T> contacts() const
	{
		contacts::Config<T> config {};

		const T athresh = static_cast<T>(this->contacts_activation_threshold);
		const T dthresh = static_cast<T>(this->contacts_deactivation_threshold);

		config.detection.normalize = true;
		config.detection.activation_threshold = athresh / static_cast<T>(255);
		config.detection.deactivation_threshold = dthresh / static_cast<T>(255);

		using Algorithm = contacts::detection::neutral::Algorithm;

		if (this->contacts_neutral == "mode")
			config.detection.neutral_value_algorithm = Algorithm::MODE;
		else if (this->contacts_neutral == "average")
			config.detection.neutral_value_algorithm = Algorithm::AVERAGE;
		else if (this->contacts_neutral == "constant")
			config.detection.neutral_value_algorithm = Algorithm::CONSTANT;

		const T nval_offset = static_cast<T>(this->contacts_neutral_value);

		config.detection.neutral_value_offset = nval_offset / static_cast<T>(255);
		config.detection.neutral_value_backoff = 16; // TODO: config option

		const T width = static_cast<T>(this->width);
		const T height = static_cast<T>(this->height);

		const T diagonal = std::hypot(width, height);

		config.validation.track_validity = true;
		config.validation.size_limits = Vector2<T> {
			static_cast<T>(this->contacts_size_min) / diagonal,
			static_cast<T>(this->contacts_size_max) / diagonal,
		};
		config.validation.aspect_limits = Vector2<f32> {
			static_cast<T>(this->contacts_aspect_min),
			static_cast<T>(this->contacts_aspect_max),
		};

		const T size_thresh = static_cast<T>(this->contacts_size_thresh);
		const T dist_thresh = static_cast<T>(this->contacts_distance_thresh);

		config.stability.check_temporal_stability = true;
		config.stability.temporal_window = this->contacts_temporal_window;
		config.stability.size_difference_threshold = size_thresh / diagonal;
		config.stability.distance_threshold = dist_thresh / diagonal;
		config.stability.movement_limits = Vector2<T> {
			static_cast<T>(this->contacts_position_thresh_min) / diagonal,
			static_cast<T>(this->contacts_position_thresh_max) / diagonal,
		};

		return config;
	}
};

} // namespace iptsd::config

#endif // IPTSD_CONFIG_CONFIG_HPP
