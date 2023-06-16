// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CORE_LINUX_SIGNAL_HANDLER_HPP
#define IPTSD_CORE_LINUX_SIGNAL_HANDLER_HPP

#include "syscalls.hpp"

#include <csignal>
#include <exception>
#include <functional>
#include <type_traits>

namespace iptsd::core::linux {

namespace impl {

template <int Signal>
class SignalStub {
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
	template <class F>
	static void setup(F &&callback)
	{
		struct sigaction sig {};

		// NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
		sig.sa_handler = SignalStub<Signal>::handler;

		// unregister handler before we replace it
		if (s_seat.m_handler)
			syscalls::sigaction(Signal, nullptr);

		// replace seat; this will unregister any old handler
		// NOTE: Omitting the template parameter does not work on Android / libc++
		s_seat.m_handler = std::function<void(int)> {std::forward<F>(callback)};

		// register new handler
		try {
			syscalls::sigaction(Signal, &sig);
		} catch (std::exception &e) {
			s_seat.m_handler = {};
			throw;
		}
	}

	/*!
	 * Removes the signal handler for the signal.
	 */
	static void clear()
	{
		if (!s_seat.m_handler)
			return;

		try {
			syscalls::sigaction(Signal, nullptr);
		} catch (std::exception &) {
			// ignored
		}

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
template <int Signal>
class SignalGuard {
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
template <int Signal, class F>
[[nodiscard]] impl::SignalGuard<Signal> signal(F &&callback)
{
	impl::SignalStub<Signal>::setup(std::forward<F>(callback));
	return {};
}

} // namespace iptsd::core::linux

#endif // IPTSD_CORE_LINUX_SIGNAL_HANDLER_HPP
