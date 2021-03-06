/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_TOUCH_MANAGER_HPP
#define IPTSD_DAEMON_TOUCH_MANAGER_HPP

#include "config.hpp"

#include <common/types.hpp>
#include <contacts/container/image.hpp>
#include <contacts/processor.hpp>
#include <ipts/parser.hpp>

#include <memory>
#include <vector>

class TouchInput {
public:
	i32 x = 0;
	i32 y = 0;
	i32 major = 0;
	i32 minor = 0;
	i32 orientation = 0;
	i32 index = 0;
	bool active = false;
};

class TouchManager {
public:
	i32 diagonal = 0;
	std::unique_ptr<iptsd::container::Image<f32>> hm;
	std::unique_ptr<iptsd::TouchProcessor> processor;

	IptsdConfig conf;
	std::vector<TouchInput> inputs;

	TouchManager(IptsdConfig conf) : conf(conf), inputs(conf.info.max_contacts) {};

	std::vector<TouchInput> &process(const IptsHeatmap &data);

private:
	void resize(u8 width, u8 height);
};

#endif /* IPTSD_DAEMON_TOUCH_MANAGER_HPP */
