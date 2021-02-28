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


#if IPTSD_HAS_STD_CPP_ATTRIBUTE(always_inline)
#define IPTSD_ALWAYS_INLINE always_inline
#elif IPTSD_HAS_CPP_ATTRIBUTE(gnu::always_inline)
#define IPTSD_ALWAYS_INLINE gnu::always_inline
#else
#define IPTSD_ALWAYS_INLINE
#endif /* IPTSD_HAS_STD_CPP_ATTRIBUTE(always_inline) */

#if IPTSD_HAS_STD_CPP_ATTRIBUTE(noinline)
#define IPTSD_NOINLINE noinline
#elif IPTSD_HAS_CPP_ATTRIBUTE(gnu::noinline)
#define IPTSD_NOINLINE gnu::noinline
#else
#define IPTSD_NOINLINE
#endif /* IPTSD_HAS_STD_CPP_ATTRIBUTE(noinline) */

#if IPTSD_HAS_STD_CPP_ATTRIBUTE(noreturn)
#define IPTSD_NORETURN noreturn
#elif IPTSD_HAS_CPP_ATTRIBUTE(gnu::noreturn)
#define IPTSD_NORETURN gnu::noreturn
#else
#define IPTSD_NORETURN
#endif /* IPTSD_HAS_STD_CPP_ATTRIBUTE(noreturn) */

#if IPTSD_HAS_STD_CPP_ATTRIBUTE(cold)
#define IPTSD_COLD cold
#elif IPTSD_HAS_CPP_ATTRIBUTE(gnu::cold)
#define IPTSD_COLD gnu::cold
#else
#define IPTSD_COLD
#endif /* IPTSD_HAS_STD_CPP_ATTRIBUTE(cold) */

#if IPTSD_HAS_STD_CPP_ATTRIBUTE(hot)
#define IPTSD_HOT hot
#elif IPTSD_HAS_CPP_ATTRIBUTE(gnu::hot)
#define IPTSD_HOT gnu::hot
#else
#define IPTSD_HOT
#endif /* IPTSD_HAS_STD_CPP_ATTRIBUTE(hot) */

#endif /* _IPTSD_COMMON_COMPILER_HPP_ */
