/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_TOUCH_HPP
#define IPTSD_DAEMON_TOUCH_HPP

#include "context.hpp"

#include <ipts/parser.hpp>

namespace iptsd::daemon {

void iptsd_touch_input(Context &ctx, const ipts::Heatmap &data);

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_TOUCH_HPP */
