/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_PROCESSOR_HPP
#define IPTSD_CONTACTS_PROCESSOR_HPP

#include "advanced/processor.hpp"
#include "basic/processor.hpp"
#include "interface.hpp"

#include <common/types.hpp>

#include <memory>

namespace iptsd::contacts {

class TouchProcessor : public ITouchProcessor {
public:
	container::Image<f32> &hm() override;
	const std::vector<TouchPoint> &process() override;

	[[nodiscard]] const eval::perf::Registry &perf() const override;

	i32 diagonal();
	void resize(index2_t size);

	Config conf;
	bool advanced = false;

private:
	i32 diag = 0;
	std::unique_ptr<ITouchProcessor> tp;
};

} /* namespace iptsd::contacts */

#endif /* IPTSD_CONTACTS_PROCESSOR_HPP */
