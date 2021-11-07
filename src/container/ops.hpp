/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_CONTAINER_OPS_HPP
#define IPTSD_CONTAINER_OPS_HPP

#include <math/num.hpp>

#include <algorithm>
#include <cassert>
#include <numeric>
#include <utility>

namespace iptsd::container::ops {

template <class C>
inline auto minmax_element(C &container) -> std::pair<typename C::iterator, typename C::iterator>
{
	return std::minmax_element(container.begin(), container.end());
}

template <class C>
inline auto minmax_element(C const &container)
	-> std::pair<typename C::const_iterator, typename C::const_iterator>
{
	return std::minmax_element(container.begin(), container.end());
}

template <class C>
inline auto minmax(C const &container) -> std::pair<typename C::value_type, typename C::value_type>
{
	auto const [min, max] = minmax_element(container);
	return {*min, *max};
}

template <class S, class T, class F> inline void transform(S const &source, T &target, F fn)
{
	assert(source.size() == target.size());

	std::transform(source.begin(), source.end(), target.begin(), fn);
}

template <class C, class F> inline void transform(C &container, F fn)
{
	std::transform(container.begin(), container.end(), container.begin(), fn);
}

template <class C> inline auto sum(C const &container) -> typename C::value_type
{
	using T = typename C::value_type;

	return std::accumulate(container.begin(), container.end(), math::num<T>::zero);
}

} /* namespace iptsd::container::ops */

#endif /* IPTSD_CONTAINER_OPS_HPP */
