/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_STYLUS_HPP_
#define _IPTSD_STYLUS_HPP_

#include "context.hpp"
#include "protocol.h"

void iptsd_stylus_handle_input(IptsdContext *iptsd, struct ipts_payload_frame frame);

#endif /* _IPTSD_STYLUS_HPP_ */
