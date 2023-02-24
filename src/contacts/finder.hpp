/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_FINDER_HPP
#define IPTSD_CONTACTS_FINDER_HPP

#include "interface.hpp"

#include <common/types.hpp>
#include <math/mat2.hpp>

#include <memory>
#include <tuple>
#include <vector>

namespace iptsd::contacts {

struct Contact {
	f64 x = 0;
	f64 y = 0;

	f64 angle = 0;
	f64 major = 0;
	f64 minor = 0;

	u32 index = 0;
	bool valid = true;
	bool stable = false;
	bool active = false;
};

enum BlobDetection {
	BASIC,
	ADVANCED,
};

struct Config {
	u32 max_contacts;
	u32 temporal_window;

	f32 width;
	f32 height;

	bool invert_x;
	bool invert_y;

	enum BlobDetection detection_mode;

	enum NeutralMode neutral_mode;
	f32 neutral_value;
	f32 activation_threshold;
	f32 deactivation_threshold;

	f32 aspect_min;
	f32 aspect_max;
	f32 size_min;
	f32 size_max;

	f32 size_thresh;
	f32 position_thresh_min;
	f32 position_thresh_max;
	f32 dist_thresh;
};

class ContactFinder {
private:
	Config config;

	index2_t size {};
	std::unique_ptr<IBlobDetector> detector = nullptr;

	std::vector<std::vector<Contact>> frames {};
	std::vector<f64> distances {};

	f64 data_diag = 0;
	f64 phys_diag = 0;

public:
	ContactFinder(Config config);

	container::Image<f32> &data();
	const std::vector<Contact> &search();

	void resize(index2_t size);
	void reset();

private:
	bool check_valid(const Contact &contact);
	bool check_dist(const Contact &from, const Contact &to);

	void track();
};

inline container::Image<f32> &ContactFinder::data()
{
	return this->detector->data();
}

} /* namespace iptsd::contacts */

#endif /* IPTSD_CONTACTS_FINDER_HPP */
