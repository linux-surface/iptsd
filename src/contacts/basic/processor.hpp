/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTACTS_BASIC_PROCESSOR_HPP
#define IPTSD_CONTACTS_BASIC_PROCESSOR_HPP

#include "heatmap.hpp"

#include <common/types.hpp>
#include <contacts/eval/perf.hpp>
#include <contacts/interface.hpp>
#include <container/image.hpp>

namespace iptsd::contacts::basic {

class TouchProcessor : public ITouchProcessor {
public:
	TouchProcessor(Config cfg);

	container::Image<f32> &hm() override;
	const std::vector<TouchPoint> &process() override;

	[[nodiscard]] const eval::perf::Registry &perf() const override;

private:
	Heatmap heatmap;
	std::vector<TouchPoint> touchpoints;

	Config cfg;
	eval::perf::Registry perfreg;
};

inline container::Image<f32> &TouchProcessor::hm()
{
	return this->heatmap.data;
}

inline const eval::perf::Registry &TouchProcessor::perf() const
{
	return this->perfreg;
}

} /* namespace iptsd::contacts::basic */

#endif /* IPTSD_CONTACTS_BASIC_PROCESSOR_HPP */
