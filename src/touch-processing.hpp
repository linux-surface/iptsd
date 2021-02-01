/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_TOUCH_PROCESSING_HPP_
#define _IPTSD_TOUCH_PROCESSING_HPP_

#include "cone.hpp"
#include "config.hpp"
#include "contact.hpp"
#include "heatmap.hpp"
#include "ipts.h"
#include "types.hpp"

#include <cstddef>
#include <vector>

class TouchInput {
public:
	i32 x;
	i32 y;
	i32 major;
	i32 minor;
	i32 orientation;
	f32 ev1;
	f32 ev2;
	i32 index;
	i32 slot;
	bool is_stable;
	bool is_palm;

	Contact *contact;

	f64 dist(TouchInput o);
	void reset(void);
};

class TouchProcessor {
public:
	Heatmap *hm;
	std::vector<Contact> contacts;
	std::vector<TouchInput> inputs;
	std::vector<TouchInput> last;
	std::vector<Cone *> rejection_cones;
	std::vector<bool> free_indices;
	std::vector<f64> distances;
	std::vector<size_t> indices;

	IptsdConfig *config;
	struct ipts_device_info info;

	TouchProcessor(struct ipts_device_info info, IptsdConfig *config);

	void process(Heatmap *hm);

private:
	void save(void);
	void update_cone(Contact *palm);
	bool check_cone(Contact *input);
	void find_palms(size_t count);
};

#endif /* _IPTSD_TOUCH_PROCESSING_HPP_ */
