/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DAEMON_DFT_HPP
#define IPTSD_DAEMON_DFT_HPP

#include "context.hpp"

#include <ipts/parser.hpp>

namespace iptsd::daemon {

void iptsd_dft_input(Context &ctx, const ipts::DftWindow &dft, ipts::StylusData &stylus);

} /* namespace iptsd::daemon */

#endif /* IPTSD_DAEMON_DFT_HPP */
