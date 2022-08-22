/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_FINDER_HPP
#define IPTSD_CONTACTS_FINDER_HPP

#include "interface.hpp"

#include <common/types.hpp>
#include <math/mat2.hpp>

#include <memory>
#include <tuple>

namespace iptsd::contacts {

struct Contact {
	f64 x = 0;
	f64 y = 0;

	f64 angle = 0;
	f64 major = 0;
	f64 minor = 0;

	u32 index = 0;
	bool palm = false;
	bool stable = false;
	bool active = false;
};

enum BlobDetection {
	BASIC,
	ADVANCED,
};

struct Config {
	u32 max_contacts;

	f32 width;
	f32 height;

	bool invert_x;
	bool invert_y;

	enum BlobDetection mode;

	f32 finger_size;
	f32 thumb_size;
	f32 thumb_aspect;
	f32 palm_aspect;

	f32 size_thresh;
	f32 position_thresh;
	f32 dist_thresh;
};

class ContactFinder {
private:
	Config config;

	index2_t size {};
	std::unique_ptr<IBlobDetector> detector = nullptr;

	std::vector<Contact> contacts {};
	std::vector<Contact> last {};
	std::vector<f64> distances {};

	f64 data_diag = 0;
	f64 phys_diag = 0;

public:
	ContactFinder(Config config);

	container::Image<f32> &data();
	const std::vector<Contact> &search();

	void resize(index2_t size);

private:
	bool check_palm(const Contact &contact);
	bool check_dist(const Contact &from, const Contact &to);

	void track();
};

inline container::Image<f32> &ContactFinder::data()
{
	return this->detector->data();
}

} /* namespace iptsd::contacts */

#endif /* IPTSD_CONTACTS_FINDER_HPP */
