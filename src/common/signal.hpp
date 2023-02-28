/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_COMMON_SIGNAL_HPP
#define IPTSD_COMMON_SIGNAL_HPP

#include "cerror.hpp"

#include <csignal>
#include <functional>
#include <type_traits>

namespace iptsd::common {
namespace detail {

template <int s> class SignalStub {
public:
	template <class F> static void setup(F &&callback);

	static void clear();

	~SignalStub();

private:
	static void handler(int signum);

	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	inline static SignalStub s_seat;

	std::function<void(int)> m_handler;
};

template <int s> SignalStub<s>::~SignalStub()
{
	if (m_handler) {
		sigaction(s, nullptr, nullptr);
		m_handler = {};
	}
}

template <int s> void SignalStub<s>::handler(int signum)
{
	s_seat.m_handler(signum);
}

template <int s> template <class F> void SignalStub<s>::setup(F &&callback)
{
	struct sigaction sig {};
	sig.sa_handler = SignalStub<s>::handler;

	// unregister handler before we replace it
	if (s_seat.m_handler) {
		const int ret = sigaction(s, nullptr, nullptr);
		if (ret == -1)
			throw iptsd::common::cerror("Failed to unregister signal handler");
	}

	// replace seat; this will unregister any old handler
	s_seat.m_handler = std::function {std::forward<F>(callback)};

	// register new handler
	const int ret = sigaction(s, &sig, nullptr);
	if (ret == -1) {
		s_seat.m_handler = {};
		throw iptsd::common::cerror("Failed to register signal handler");
	}
}

template <int s> void SignalStub<s>::clear()
{
	if (s_seat.m_handler) {
		sigaction(s, nullptr, nullptr);
		s_seat.m_handler = {};
	}
}

template <int s> class SignalGuard {
public:
	~SignalGuard();
};

template <int s> SignalGuard<s>::~SignalGuard()
{
	SignalStub<s>::clear();
}

} /* namespace detail */

template <int s, class F> [[nodiscard]] detail::SignalGuard<s> signal(F &&callback)
{
	detail::SignalStub<s>::setup(std::forward<F>(callback));
	return {};
}

} /* namespace iptsd::common */

#endif /* IPTSD_COMMON_SIGNAL_HPP */
