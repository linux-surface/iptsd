/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_COMMON_COMPILER_HPP
#define IPTSD_COMMON_COMPILER_HPP

#define likely(X)   __builtin_expect(!!(X), 1) /* NOLINT(cppcoreguidelines-macro-usage) */
#define unlikely(X) __builtin_expect(!!(X), 0) /* NOLINT(cppcoreguidelines-macro-usage) */

#endif /* IPTSD_COMMON_COMPILER_HPP */
