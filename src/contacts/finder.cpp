// SPDX-License-Identifier: GPL-2.0-or-later

#include "finder.hpp"

#include "advanced/detector.hpp"
#include "basic/detector.hpp"

#include <container/image.hpp>
#include <math/mat2.hpp>

#include <algorithm>
#include <gsl/gsl>
#include <memory>
#include <vector>

namespace iptsd::contacts {

ContactFinder::ContactFinder(Config config) : config {config}
{
	this->contacts.resize(config.max_contacts);
	this->last.resize(config.max_contacts);
	this->distances.resize(static_cast<std::size_t>(config.max_contacts) * config.max_contacts);
}

void ContactFinder::resize(index2_t size)
{
	if (this->detector && this->size == size)
		return;

	this->detector.reset();
	this->size = size;

	if (this->config.mode == BlobDetection::BASIC)
		this->detector = std::make_unique<basic::BlobDetector>(size);
	else if (this->config.mode == BlobDetection::ADVANCED)
		this->detector = std::make_unique<advanced::BlobDetector>(size);
}

bool ContactFinder::check_palm(const Contact &contact)
{
	// TODO: reimplement
	return false;
}

bool ContactFinder::check_dist(const Contact &from, const Contact &to)
{
	// TODO: reimplement
	return false;
}

const std::vector<Contact> &ContactFinder::search()
{
	f64 data_diag = std::sqrt(this->size.x * this->size.x + this->size.y * this->size.y);
	f64 phys_diag = std::sqrt(this->config.width * this->config.width +
				  this->config.height * this->config.height);

	const std::vector<Blob> &blobs = this->detector->search();
	std::size_t count = std::min(blobs.size(), static_cast<u64>(this->config.max_contacts));

	for (std::size_t i = 0; i < count; i++) {
		const auto &blob = blobs[i];
		auto &contact = this->contacts[i];

		f64 x = blob.mean.x;
		f64 y = blob.mean.y;

		if (this->config.invert_x)
			x = 1 - x;

		if (this->config.invert_y)
			y = 1 - y;

		// Convert from relative to physical dimensions
		contact.x = x * this->config.width;
		contact.y = y * this->config.height;

		math::Eigen2<f32> eigen = blob.cov.eigen();
		f64 s1 = std::sqrt(eigen.w[0]);
		f64 s2 = std::sqrt(eigen.w[1]);

		f64 d1 = s1 / data_diag;
		f64 d2 = s2 / data_diag;

		contact.major = std::max(d1, d2) * phys_diag;
		contact.minor = std::min(d1, d2) * phys_diag;

		math::Vec2<f64> v1 = eigen.v[0].cast<f64>() * s1;
		f64 angle = (math::num<f64>::pi / 2) - std::atan2(v1.x, v1.y);

		// Make sure that the angle is always a positive number
		if (angle < 0)
			angle += math::num<f64>::pi;
		else if (angle > math::num<f64>::pi)
			angle -= math::num<f64>::pi;

		contact.angle = angle;
		contact.eigen = eigen;

		contact.palm = this->check_palm(contact);
		contact.stable = true;

		contact.index = i;
		contact.active = true;
	}

	for (std::size_t i = count; i < this->config.max_contacts; i++) {
		auto &contact = this->contacts[i];

		contact.index = i;
		contact.active = false;
		contact.palm = false;
		contact.eigen = math::Eigen2<f32> {};
	}

	// Mark contacts that are very close to a palm as palms too
	for (const auto &contact : this->contacts) {
		if (!contact.palm)
			continue;

		for (auto &other : this->contacts) {
			if (!this->check_dist(contact, other))
				continue;

			other.palm = true;
		}
	}

	this->track();

	// Convert all values back to relative dimensions
	for (std::size_t i = 0; i < count; i++) {
		auto &contact = this->contacts[i];

		contact.x /= this->config.width;
		contact.y /= this->config.height;

		contact.major /= phys_diag;
		contact.minor /= phys_diag;
	}

	std::swap(this->contacts, this->last);
	return this->last;
}

void ContactFinder::track()
{
	f64 position_thresh_sq = this->config.position_thresh * this->config.position_thresh;

	// Calculate the distances between current and previous inputs
	for (u32 i = 0; i < this->config.max_contacts; i++) {
		for (u32 j = 0; j < this->config.max_contacts; j++) {
			const auto &in = this->contacts[i];
			const auto &last = this->last[j];

			u32 idx = i * this->config.max_contacts + j;

			// If one of the two inputs is / was not active, generate
			// a very high distance, so that the pair will only get
			// chosen if no "proper" pairs are left.
			if (!in.active || !last.active) {
				this->distances[idx] = (1 << 20) + idx;
				continue;
			}

			f64 dx = in.x - last.x;
			f64 dy = in.y - last.y;

			this->distances[idx] = std::sqrt(dx * dx + dy * dy);
		}
	}

	// Select the smallest calculated distance to find the closest two inputs.
	// Copy the index from the previous to the current input. Then invalidate
	// all distance entries that contain the two inputs, and repeat until we
	// found an index for all inputs.
	for (u32 k = 0; k < this->config.max_contacts; k++) {
		auto it = std::min_element(this->distances.begin(), this->distances.end());
		u32 idx = std::distance(this->distances.begin(), it);

		u32 i = idx / this->config.max_contacts;
		u32 j = idx % this->config.max_contacts;

		auto &contact = this->contacts[i];
		const auto &last = this->last[j];

		contact.index = last.index;
		if (contact.active)
			contact.palm |= last.palm;

		f64 dmaj = contact.major - last.major;
		f64 dmin = contact.minor - last.minor;

		// Is the contact rapidly increasing its size?
		contact.stable =
			(dmaj < this->config.size_thresh && dmin < this->config.size_thresh);

		f64 dx = contact.x - last.x;
		f64 dy = contact.y - last.y;
		f64 sqdist = dx * dx + dy * dy;

		// Is the position stable?
		if (sqdist < position_thresh_sq) {
			contact.x = last.x;
			contact.y = last.y;
		} else {
			f64 dist = std::sqrt(sqdist);

			contact.x -= this->config.position_thresh * (dx / dist);
			contact.y -= this->config.position_thresh * (dy / dist);
		}

		// Set the distance of all pairs that contain one of i and j
		// to something even higher than the distance chosen above.
		// This prevents i and j from getting selected again.
		for (u32 x = 0; x < this->config.max_contacts; x++) {
			u32 idx1 = i * this->config.max_contacts + x;
			u32 idx2 = x * this->config.max_contacts + j;

			this->distances[idx1] = (1 << 30) + idx1;
			this->distances[idx2] = (1 << 30) + idx2;
		}
	}
}

} // namespace iptsd::contacts
