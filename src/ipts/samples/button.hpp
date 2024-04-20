// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_IPTS_SAMPLES_BUTTON_HPP
#define IPTSD_IPTS_SAMPLES_BUTTON_HPP

#include <common/types.hpp>

namespace iptsd::ipts::samples {

struct Button {
	//! Whether the user is pressing hard enough on the button to activate it.
	bool active = false;

	//! How hard the user is pressing on the button.
	//! Range: 0 to 1
	f64 pressure = 0;
};

} // namespace iptsd::ipts::samples

#endif // IPTSD_IPTS_SAMPLES_BUTTON_HPP
