/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_TOUCH_MANAGER_HPP
#define IPTSD_DAEMON_TOUCH_MANAGER_HPP

#include "cone.hpp"
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
	bool palm = false;
	bool stable = false;
	bool active = false;

	f32 ev1 = 0;
	f32 ev2 = 0;
};

class TouchManager {
public:
	i32 diagonal = 0;
	index2_t size;
	contacts::TouchProcessor processor;

	Config conf;

	std::vector<TouchInput> inputs;

	std::vector<TouchInput> last;
	std::vector<f64> distances;

	std::vector<std::shared_ptr<Cone>> cones;

	TouchManager(Config conf);

	std::vector<TouchInput> &process(const ipts::Heatmap &data);

private:
	void track();
	void update_cones(const TouchInput &palm);
	bool check_cones(const TouchInput &input);
};

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_TOUCH_MANAGER_HPP */
