/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_STYLUS_HPP
#define IPTSD_DAEMON_STYLUS_HPP

#include "context.hpp"

#include <ipts/parser.hpp>

namespace iptsd::daemon {

void iptsd_stylus_input(Context &ctx, const ipts::StylusData &data);

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_STYLUS_HPP */
