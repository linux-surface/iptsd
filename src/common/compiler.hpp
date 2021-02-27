/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_COMMON_COMPILER_HPP_
#define _IPTSD_COMMON_COMPILER_HPP_

#ifdef __has_cpp_attribute
#define IPTSD_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define IPTSD_HAS_CPP_ATTRIBUTE(x) 0
#endif

#define IPTSD_HAS_STD_CPP_ATTRIBUTE(x) \
	(IPTSD_HAS_CPP_ATTRIBUTE(x) > 1 && IPTSD_HAS_CPP_ATTRIBUTE(x) <= __cplusplus)


#if IPTSD_HAS_STD_CPP_ATTRIBUTE(likely)
#define IPTSD_LIKELY likely
#elif IPTSD_HAS_CPP_ATTRIBUTE(gnu::likely)
#define IPTSD_LIKELY gnu::likely
#else
#define IPTSD_LIKELY
#endif /* IPTSD_HAS_STD_CPP_ATTRIBUTE(likely) */

#if IPTSD_HAS_STD_CPP_ATTRIBUTE(unlikely)
#define IPTSD_UNLIKELY unlikely
#elif IPTSD_HAS_CPP_ATTRIBUTE(gnu::unlikely)
#define IPTSD_UNLIKELY gnu::unlikely
#else
#define IPTSD_UNLIKELY
#endif /* IPTSD_HAS_STD_CPP_ATTRIBUTE(unlikely) */

#endif /* _IPTSD_COMMON_COMPILER_HPP_ */
