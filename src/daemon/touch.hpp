/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_TOUCH_HPP
#define IPTSD_DAEMON_TOUCH_HPP

#include "context.hpp"

#include <ipts/parser.hpp>

void iptsd_touch_input(IptsdContext &ctx, const IptsHeatmap &data);

#endif /* IPTSD_DAEMON_TOUCH_HPP */
