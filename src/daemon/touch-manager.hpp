/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_DAEMON_TOUCH_MANAGER_HPP_
#define _IPTSD_DAEMON_TOUCH_MANAGER_HPP_

#include "config.hpp"

#include <common/types.hpp>
#include <contacts/container/image.hpp>
#include <contacts/processor.hpp>
#include <ipts/parser.hpp>

#include <vector>

class TouchInput {
public:
	i32 x;
	i32 y;
	i32 major;
	i32 minor;
	i32 orientation;
	i32 index;
	bool active;
};

class TouchManager {
public:
	i32 diagonal;
	iptsd::container::Image<f32> *hm;
	iptsd::TouchProcessor *processor;

	IptsdConfig conf;
	std::vector<TouchInput> inputs;

	TouchManager(IptsdConfig conf);
	~TouchManager(void);

	std::vector<TouchInput> &process(IptsHeatmap data);

private:
	void resize(u8 width, u8 height);
};

#endif /* _IPTSD_DAEMON_TOUCH_MANAGER_HPP_ */
