/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_TOUCH_PROCESSING_HPP_
#define _IPTSD_TOUCH_PROCESSING_HPP_

#include "cone.hpp"
#include "config.hpp"
#include "contact.hpp"
#include "heatmap.hpp"
#include "ipts.h"

#include <cstddef>
#include <cstdint>
#include <vector>

class TouchInput {
public:
	int32_t x;
	int32_t y;
	int32_t major;
	int32_t minor;
	int32_t orientation;
	float ev1;
	float ev2;
	int32_t index;
	int32_t slot;
	bool is_stable;
	bool is_palm;

	Contact *contact;

	double dist(TouchInput o);
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
	std::vector<double> distances;
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
