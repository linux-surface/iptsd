/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_COMMON_COMPILER_HPP
#define IPTSD_COMMON_COMPILER_HPP

#define likely(X)   __builtin_expect(!!(X), 1)
#define unlikely(X) __builtin_expect(!!(X), 0)

#endif /* IPTSD_COMMON_COMPILER_HPP */
