// SPDX-License-Identifier: GPL-2.0-or-later

#include "finder.hpp"

#include "advanced/detector.hpp"
#include "basic/detector.hpp"
#include "interface.hpp"

#include <container/image.hpp>
#include <math/mat2.hpp>

#include <algorithm>
#include <cmath>
#include <gsl/gsl>
#include <memory>
#include <stdexcept>
#include <vector>

namespace iptsd::contacts {

ContactFinder::ContactFinder(Config config)
	: config {config}, phys_diag(std::hypot(config.width, config.height))
{
	this->distances.resize(static_cast<std::size_t>(config.max_contacts) * config.max_contacts);

	// The temporal window must at least be 2 for finger tracking to work
	if (config.temporal_window < 2)
		throw std::runtime_error("The temporal window must at least be two!");

	for (std::size_t i = 0; i < config.temporal_window; i++) {
		std::vector<Contact> frame(config.max_contacts);

		// Make sure that the contacts in last have proper indices set,
		// to avoid copying the index 0 to all contacts.
		for (std::size_t j = 0; j < frame.size(); j++) {
			frame[j].index = j;
			frame[j].active = false;
		}

		this->frames.push_back(std::move(frame));
	}
}

void ContactFinder::resize(index2_t size)
{
	if (this->detector && this->size == size)
		return;

	this->detector.reset();
	this->size = size;
	this->data_diag = std::hypot(size.x, size.y);

	BlobDetectorConfig config {};
	config.neutral_mode = this->config.neutral_mode;
	config.neutral_value = this->config.neutral_value;
	config.activation_threshold = this->config.activation_threshold;
	config.deactivation_threshold = this->config.deactivation_threshold;

	if (this->config.detection_mode == BlobDetection::BASIC)
		this->detector = std::make_unique<basic::BlobDetector>(size, config);
	else if (this->config.detection_mode == BlobDetection::ADVANCED)
		this->detector = std::make_unique<advanced::BlobDetector>(size, config);
}

void ContactFinder::reset()
{
	for (std::size_t i = 0; i < config.temporal_window; i++) {
		const std::size_t size = this->frames[i].size();

		for (std::size_t j = 0; j < size; j++)
			this->frames[i][j].active = false;
	}
}

bool ContactFinder::check_valid(const Contact &contact)
{
	const f64 aspect = contact.major / contact.minor;
	const f64 major = contact.major * this->phys_diag;

	if (aspect < this->config.aspect_min || aspect > this->config.aspect_max)
		return false;

	if (major < this->config.size_min || major > this->config.size_max)
		return false;

	return true;
}

bool ContactFinder::check_dist(const Contact &from, const Contact &to)
{
	// Assumption: All contacts are perfect circles. The radius is major / 2.
	// This might cover a bit more area than neccessary, but will make implementation easier.

	const f64 dx = (from.x - to.x) * this->config.width;
	const f64 dy = (from.y - to.y) * this->config.height;

	f64 dist = std::hypot(dx, dy);
	dist -= (from.major * this->phys_diag) / 2;
	dist -= (to.major * this->phys_diag) / 2;

	return dist <= this->config.dist_thresh;
}

const std::vector<Contact> &ContactFinder::search()
{
	const std::vector<Blob> &blobs = this->detector->search();
	const std::size_t count =
		std::min(blobs.size(), static_cast<u64>(this->config.max_contacts));

	for (std::size_t i = 0; i < count; i++) {
		const Blob &blob = blobs[i];
		Contact &contact = this->frames[0][i];

		contact.x = blob.mean.x / gsl::narrow<f32>(this->size.x);
		contact.y = blob.mean.y / gsl::narrow<f32>(this->size.y);

		if (this->config.invert_x)
			contact.x = 1 - contact.x;

		if (this->config.invert_y)
			contact.y = 1 - contact.y;

		const math::Eigen2<f64> eigen = blob.cov.eigen();
		const f64 s1 = std::sqrt(eigen.w[0]);
		const f64 s2 = std::sqrt(eigen.w[1]);

		const f64 d1 = s1 / this->data_diag;
		const f64 d2 = s2 / this->data_diag;

		contact.major = std::max(d1, d2);
		contact.minor = std::min(d1, d2);

		// The eigenvalues give us the radius of the ellipse, but
		// major / minor need to be the full length of the axis.
		contact.major *= 2;
		contact.minor *= 2;

		const math::Vec2<f64> v = eigen.v[0].cast<f64>() * s1;
		f64 angle = std::atan2(v.x, v.y) + (math::num<f64>::pi / 2);

		// It is not possible to say if the contact faces up or down,
		// so we make sure the angle is between 0° and 180° to be consistent
		if (angle < 0)
			angle += math::num<f64>::pi;
		else if (angle >= math::num<f64>::pi)
			angle -= math::num<f64>::pi;

		if (this->config.invert_x != this->config.invert_y)
			angle = math::num<f64>::pi - angle;

		contact.angle = angle;

		contact.valid = this->check_valid(contact);
		contact.stable = true;

		contact.index = i;
		contact.active = true;
	}

	for (std::size_t i = count; i < this->config.max_contacts; i++) {
		Contact &contact = this->frames[0][i];

		contact.index = i;
		contact.active = false;
		contact.valid = true;
		contact.x = 0;
		contact.y = 0;
		contact.angle = 0;
		contact.major = 0;
		contact.minor = 0;
	}

	// Mark contacts that are very close to an invalid contact as invalid too
	for (const Contact &contact : this->frames[0]) {
		if (contact.valid)
			continue;

		for (Contact &other : this->frames[0]) {
			if (!this->check_dist(contact, other))
				continue;

			other.stable = false;
		}
	}

	this->track();

	for (std::size_t i = config.temporal_window - 1; i > 0; i--)
		std::swap(this->frames[i], this->frames[i - 1]);

	return this->frames[1];
}

void ContactFinder::track()
{
	// Calculate the distances between current and previous inputs
	for (u32 i = 0; i < this->config.max_contacts; i++) {
		for (u32 j = 0; j < this->config.max_contacts; j++) {
			const Contact &in = this->frames[0][i];
			const Contact &last = this->frames[1][j];

			const u32 idx = i * this->config.max_contacts + j;

			// If one of the two inputs is / was not active, generate
			// a very high distance, so that the pair will only get
			// chosen if no "proper" pairs are left.
			if (!in.active || !last.active) {
				this->distances[idx] = (1 << 20) + idx;
				continue;
			}

			const f64 dx = in.x - last.x;
			const f64 dy = in.y - last.y;

			this->distances[idx] = std::hypot(dx, dy);
		}
	}

	// Select the smallest calculated distance to find the closest two inputs.
	// Copy the index from the previous to the current input. Then invalidate
	// all distance entries that contain the two inputs, and repeat until we
	// found an index for all inputs.
	for (u32 k = 0; k < this->config.max_contacts; k++) {
		const auto it = std::min_element(this->distances.begin(), this->distances.end());
		const u32 idx = std::distance(this->distances.begin(), it);

		const u32 i = idx / this->config.max_contacts;
		const u32 j = idx % this->config.max_contacts;

		Contact &contact = this->frames[0][i];
		const Contact &last = this->frames[1][j];

		contact.index = last.index;
		if (contact.active)
			contact.valid &= last.valid;

		const f64 dmaj = (contact.major - last.major) * this->phys_diag;
		const f64 dmin = (contact.minor - last.minor) * this->phys_diag;

		// Is the contact rapidly increasing its size?
		contact.stable =
			(dmaj < this->config.size_thresh && dmin < this->config.size_thresh);

		// Check if there was an active contact with the same index in all stored frames.
		// If this is not the case the contact is not temporally stable and will be ignored.
		for (std::size_t i = 1; i < this->frames.size(); i++) {
			for (const Contact &c : this->frames.at(i)) {
				if (contact.index != c.index)
					continue;

				if (contact.active && !c.active)
					contact.stable = false;
			}
		}

		const f64 dx = (contact.x - last.x) * this->config.width;
		const f64 dy = (contact.y - last.y) * this->config.height;
		const f64 dist = std::hypot(dx, dy);

		if (contact.active && last.active) {
			if (dist > this->config.position_thresh_max) {
				// Mark large position changes as unstable to prevent contacts
				// jumping away in one frame and jumping back in the next one.
				contact.stable = false;
			} else if (dist < this->config.position_thresh_min) {
				// Ignore small position changes to prevent jitter
				contact.x = last.x;
				contact.y = last.y;
			} else {
				// Stabilize movements
				contact.x -= (this->config.position_thresh_min * (dx / dist)) /
					     this->config.width;
				contact.y -= (this->config.position_thresh_min * (dy / dist)) /
					     this->config.height;
			}
		}

		// Don't mark unstable contacts as invalid, because the true state is not known yet.
		if (!contact.stable)
			contact.valid = true;

		// Set the distance of all pairs that contain one of i and j
		// to something even higher than the distance chosen above.
		// This prevents i and j from getting selected again.
		for (u32 x = 0; x < this->config.max_contacts; x++) {
			const u32 idx1 = i * this->config.max_contacts + x;
			const u32 idx2 = x * this->config.max_contacts + j;

			this->distances[idx1] = (1 << 30) + idx1;
			this->distances[idx2] = (1 << 30) + idx2;
		}
	}
}

} // namespace iptsd::contacts
