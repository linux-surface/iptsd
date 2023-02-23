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
		for (u32 j = 0; j < frame.size(); j++) {
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

bool ContactFinder::check_valid(const Contact &contact)
{
	f64 aspect = contact.major / contact.minor;
	f64 major = contact.major * this->phys_diag;

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

	f64 dx = (from.x - to.x) * this->config.width;
	f64 dy = (from.y - to.y) * this->config.height;

	f64 dist = std::hypot(dx, dy);
	dist -= (from.major * this->phys_diag) / 2;
	dist -= (to.major * this->phys_diag) / 2;

	return dist <= this->config.dist_thresh;
}

const std::vector<Contact> &ContactFinder::search()
{
	const std::vector<Blob> &blobs = this->detector->search();
	u32 count = std::min(gsl::narrow_cast<u32>(blobs.size()), this->config.max_contacts);
    
    u32 actual_cnt = count;
    u32 palm_cnt = 0;

	for (u32 i = 0; i < actual_cnt; i++) {
		const auto &blob = blobs[i+palm_cnt];
		auto &contact = this->frames[0][i];

		contact.x = blob.mean.x / gsl::narrow<f32>(this->size.x);
		contact.y = blob.mean.y / gsl::narrow<f32>(this->size.y);

		if (this->config.invert_x)
			contact.x = 1 - contact.x;

		if (this->config.invert_y)
			contact.y = 1 - contact.y;

		math::Eigen2<f64> eigen = blob.cov.eigen();
		f64 s1 = std::sqrt(eigen.w[0]);
		f64 s2 = std::sqrt(eigen.w[1]);

		f64 d1 = s1 / this->data_diag;
		f64 d2 = s2 / this->data_diag;

		contact.major = std::max(d1, d2);
		contact.minor = std::min(d1, d2);

		// The eigenvalues give us the radius of the ellipse, but
		// major / minor need to be the full length of the axis.
		contact.major *= 2;
		contact.minor *= 2;

		math::Vec2<f64> v = eigen.v[0].cast<f64>() * s1;
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
        
        contact.index = i;
        contact.active = true;
        
        contact.instability = 0;
        contact.tracked = false;

        contact.valid = this->check_valid(contact);
        if (!contact.valid) {   // put invalid touch (palm) to the end
            i--;
            palm_cnt++;
            std::swap(contact, this->frames[0][--actual_cnt]);
        }
	}

	for (u32 i = count; i < this->config.max_contacts; i++) {
		auto &contact = this->frames[0][i];

		contact.index = i;
		contact.active = false;
		contact.valid = true;
		contact.x = 0;
		contact.y = 0;
		contact.angle = 0;
		contact.major = 0;
		contact.minor = 0;
        
        contact.instability = 0;
        contact.tracked = false;
	}

	// Mark contacts that are very close to an invalid contact as unstable
    for (u32 i = actual_cnt; i < count; i++) {
        const auto &contact = this->frames[0][i];
        for (u32 j = 0; j < actual_cnt; j++) {
            auto &other = this->frames[0][j];
			if (!this->check_dist(contact, other))
				continue;

            other.instability++;
            if (j != actual_cnt-1) {
                std::swap(other, this->frames[0][actual_cnt-1]);
                j--;
            }
            actual_cnt--;
		}
	}

    if (touching)
        this->track(actual_cnt);
    touching = actual_cnt > 0;

    for (std::size_t i = config.temporal_window - 1; i > 0; i--)
        std::swap(this->frames[i], this->frames[i - 1]);
    last_touch_cnt = actual_cnt;

	return this->frames[1];
}

void ContactFinder::track(u32 &touch_cnt)
{
    // Delete last instable touch inputs
    for (u32 j = 0; j < this->last_touch_cnt; j++) {
        if (this->frames[1][j].instability >= this->config.instability_tolerance) {
            if (j != this->last_touch_cnt-1) {
                std::swap(this->frames[1][j], this->frames[1][this->last_touch_cnt-1]);
                j--;
            }
            this->last_touch_cnt--;
        }
    }
	// Calculate the distances between current and previous valid inputs
	for (u32 i = 0; i < touch_cnt; i++) {
		for (u32 j = 0; j < this->last_touch_cnt; j++) {
			const auto &in = this->frames[0][i];
			const auto &last = this->frames[1][j];

			u32 idx = i * this->last_touch_cnt + j;

			f64 dx = in.x - last.x;
			f64 dy = in.y - last.y;

			this->distances[idx] = std::hypot(dx, dy);
		}
	}

	// Select the smallest calculated distance to find the closest two inputs.
	// Copy the index from the previous to the current input. Then invalidate
	// all distance entries that contain the two inputs, and repeat until we
	// found an index for all inputs.
    u32 count = std::min(touch_cnt, this->last_touch_cnt);
    u16 idx_used = 0;
	for (u32 k = 0; k < count; k++) {
		auto it = std::min_element(this->distances.begin(), this->distances.begin() + touch_cnt*last_touch_cnt);
		u32 idx = (u32)std::distance(this->distances.begin(), it);

		u32 i = idx / this->last_touch_cnt;
		u32 j = idx % this->last_touch_cnt;

		auto &contact = this->frames[0][i];
		const auto &last = this->frames[1][j];

        idx_used |= 1 << last.index;
        
        contact.index = last.index;
        contact.instability += last.instability;
        contact.tracked = true;

		f64 dmaj = (contact.major - last.major) * this->phys_diag;
		f64 dmin = (contact.minor - last.minor) * this->phys_diag;

		// Is the contact rapidly increasing its size?
		if (dmaj >= this->config.size_thresh || dmin >= this->config.size_thresh)
            contact.instability+=2;

		f64 dx = (contact.x - last.x) * this->config.width;
		f64 dy = (contact.y - last.y) * this->config.height;
		f64 dist = std::hypot(dx, dy);

        if (dist > this->config.position_thresh_max) {
            // Mark large position changes as unstable to prevent contacts
            // jumping away in one frame and jumping back in the next one.
            contact.instability++;
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
        
        // if current contact is stable, clear the instability counter
        if (contact.instability == last.instability)
            contact.instability = 0;

		// Set the distance of all pairs that contain one of i and j
		// to something even higher than the distance chosen above.
		// This prevents i and j from getting selected again.
        for (u32 x = 0; x < last_touch_cnt; x++) {
            u32 idx1 = i * last_touch_cnt + x;
            this->distances[idx1] = 1 << 30;
        }
        for (u32 x = 0; x < touch_cnt; x++) {
            u32 idx2 = x * last_touch_cnt + j;
            this->distances[idx2] = 1 << 30;
        }
	}
    
    if (touch_cnt > this->last_touch_cnt) { // some finger added
        int index = -1;
        for (u32 i = 0; i < touch_cnt; i++) {
            auto &contact = this->frames[0][i];
            if (!contact.tracked) {
                while (idx_used & (1 << ++index));
                contact.index = index;
                idx_used |= 1 << index;
                index++;
            }
        }
    } else if (touch_cnt < this->last_touch_cnt) {  // Some finger lifted
        for (u32 j = 0; j < this->last_touch_cnt; j++) {
            if (!(idx_used & (1 << this->frames[1][j].index))) {
                for (u32 i = touch_cnt; i < this->config.max_contacts; i++) {
                    if (!this->frames[0][i].active) {
                        if (i != touch_cnt)
                            std::swap(this->frames[0][touch_cnt], this->frames[0][i]);
                        this->frames[0][touch_cnt] = this->frames[1][j];
                        this->frames[0][touch_cnt].instability++;
                        touch_cnt++;
                        break;
                    }
                }
            }
        }
    }
}

} // namespace iptsd::contacts
