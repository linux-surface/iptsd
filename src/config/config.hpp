/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONFIG_CONFIG_HPP
#define IPTSD_CONFIG_CONFIG_HPP

#include <common/types.hpp>
#include <contacts/finder.hpp>

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
	bool touch_check_cone = true;
	bool touch_check_stability = true;
	bool touch_disable_on_palm = false;
	bool touch_disable_on_stylus = false;

	// [Contacts]
	std::string contacts_detection = "basic";
	f32 contacts_finger_size = 1;
	f32 contacts_thumb_size = 2;
	f32 contacts_thumb_aspect = 1.5;
	f32 contacts_palm_aspect = 2;
	f32 contacts_size_thresh = 0.1;
	f32 contacts_position_thresh = 0.2;
	f32 contacts_distance_thresh = 1;

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
	Config(i16 vendor, i16 product);

	[[nodiscard]] contacts::Config contacts() const;

private:
	void load_dir(const std::string &name);
};

} /* namespace iptsd::config */

#endif /* IPTSD_CONFIG_CONFIG_HPP */
