/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_SINGLETOUCH_HPP
#define IPTSD_DAEMON_SINGLETOUCH_HPP

#include "context.hpp"

#include <ipts/parser.hpp>

namespace iptsd::daemon {

void iptsd_singletouch_input(Context &ctx, const ipts::SingletouchData &data);

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_SINGLETOUCH_HPP */
