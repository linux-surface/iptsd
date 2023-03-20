// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_COMMON_SIGNAL_HPP
#define IPTSD_COMMON_SIGNAL_HPP

#include "cerror.hpp"

#include <csignal>
#include <functional>
#include <type_traits>

namespace iptsd::common {

namespace impl {

template <int Signal> class SignalStub {
private:
	// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
	inline static SignalStub s_seat;

	// The handler function for the signal.
	std::function<void(int)> m_handler;

public:
	~SignalStub()
	{
		if (!m_handler)
			return;

		sigaction(Signal, nullptr, nullptr);
		m_handler = {};
	}

	/*!
	 * Sets up a signal handler.
	 *
	 * @param[in] callback The user defined function to call when the signal is received.
	 */
	template <class F> static void setup(F &&callback)
	{
		struct sigaction sig {};

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
		sig.sa_handler = SignalStub<Signal>::handler;

		// unregister handler before we replace it
		if (s_seat.m_handler) {
			const int ret = sigaction(Signal, nullptr, nullptr);
			if (ret == -1)
				throw common::cerror("Failed to unregister signal handler");
		}

		// replace seat; this will unregister any old handler
		s_seat.m_handler = std::function {std::forward<F>(callback)};

		// register new handler
		const int ret = sigaction(Signal, &sig, nullptr);
		if (ret == -1) {
			s_seat.m_handler = {};
			throw common::cerror("Failed to register signal handler");
		}
	}

	/*!
	 * Removes the signal handler for the signal.
	 */
	static void clear()
	{
		if (!s_seat.m_handler)
			return;

		sigaction(Signal, nullptr, nullptr);
		s_seat.m_handler = {};
	}

private:
	/*!
	 * Forward calls to the user specified signal handler.
	 */
	static void handler(const int signum)
	{
		s_seat.m_handler(signum);
	}
};

/*
 * Clears a signal handler once it moves out of scope.
 */
template <int Signal> class SignalGuard {
public:
	~SignalGuard()
	{
		SignalStub<Signal>::clear();
	}
};

} // namespace impl

/*!
 * Registers a user defined signal handler.
 *
 * @tparam Signal The signal that should be handled differently.
 * @param[in] callback The user defined function that is called when the signal is received.
 * @return A guard object that will remove the signal handler once it moves out of scope.
 */
template <int Signal, class F> [[nodiscard]] impl::SignalGuard<Signal> signal(F &&callback)
{
	impl::SignalStub<Signal>::setup(std::forward<F>(callback));
	return {};
}

} // namespace iptsd::common

#endif // IPTSD_COMMON_SIGNAL_HPP
