/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_TOUCH_MANAGER_HPP
#define IPTSD_DAEMON_TOUCH_MANAGER_HPP

#include "config.hpp"

#include <common/types.hpp>
#include <contacts/processor.hpp>
#include <container/image.hpp>
#include <ipts/parser.hpp>

#include <memory>
#include <vector>

namespace iptsd::daemon {

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
	index2_t size;
	contacts::TouchProcessor processor;

	Config conf;
	u8 max_contacts;

	std::vector<TouchInput> inputs;

	std::vector<TouchInput> last;
	std::vector<f64> distances;

	TouchManager(Config conf);

	std::vector<TouchInput> &process(const ipts::Heatmap &data);

private:
	void track();
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_TOUCH_MANAGER_HPP */
