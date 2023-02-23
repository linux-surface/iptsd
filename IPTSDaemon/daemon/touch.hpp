/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_TOUCH_HPP
#define IPTSD_DAEMON_TOUCH_HPP

#include "context.hpp"

#include <ipts/parser.hpp>

#include <spdlog/spdlog.h>

namespace iptsd::daemon {

bool iptsd_touch_input(Context &ctx, const ipts::Heatmap &data, IPTSHIDReport &report);

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_TOUCH_HPP */
