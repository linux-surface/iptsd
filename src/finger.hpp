/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_FINGER_HPP_
#define _IPTSD_FINGER_HPP_

#include "touch-processing.hpp"

#include <cstddef>

void iptsd_finger_track(TouchProcessor *tp, size_t count);

#endif /* _IPTSD_FINGER_HPP_ */
