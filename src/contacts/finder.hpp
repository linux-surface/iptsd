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
	u32 max_contacts = 0;

	f32 width;
	f32 height;

	bool invert_x = false;
	bool invert_y = false;

	enum BlobDetection mode = BlobDetection::BASIC;

	f32 size_thresh = 0.1;
	f32 position_thresh = 0.2;
};

class ContactFinder {
private:
	Config config;

	index2_t size {};
	std::unique_ptr<IBlobDetector> detector = nullptr;

	std::vector<Contact> contacts {};
	std::vector<Contact> last {};
	std::vector<f64> distances {};

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
