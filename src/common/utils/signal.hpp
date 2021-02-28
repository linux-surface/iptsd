/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_COMMON_UTILS_SIGNAL_HPP_
#define _IPTSD_COMMON_UTILS_SIGNAL_HPP_

namespace iptsd::utils {

void signal(int signum, void (*handler)(int));

} /* namespace iptsd::utils */

#endif /* _IPTSD_COMMON_UTILS_SIGNAL_HPP_ */
