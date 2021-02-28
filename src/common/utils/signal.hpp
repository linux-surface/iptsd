/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _IPTSD_COMMON_UTILS_SIGNAL_HPP_
#define _IPTSD_COMMON_UTILS_SIGNAL_HPP_

#include "cerror.hpp"

#include <csignal>
#include <functional>
#include <type_traits>


namespace iptsd::utils {
namespace detail {

template<int s>
class SignalStub {
public:
	template<class F>
	static auto setup(F&& callback);

	static void clear();

	~SignalStub();

private:
	static void handler(int signum);
	inline static SignalStub s_seat;

	std::function<void(int)> m_handler;
};

template<int s>
SignalStub<s>::~SignalStub()
{
	sigaction(s, nullptr, nullptr);
	m_handler = {};
}

template<int s>
void SignalStub<s>::handler(int signum)
{
	s_seat.m_handler(signum);
}

template<int s>
template<class F>
auto SignalStub<s>::setup(F&& callback)
{
	struct sigaction sig {};
	sig.sa_handler = SignalStub<s>::handler;

	// unregister handler before we replace it
	int ret = sigaction(s, nullptr, nullptr);
	if (ret == -1)
		throw iptsd::utils::cerror("Failed to unregister signal handler");

	// replace seat; this will unregister any old handler
	s_seat.m_handler = std::function { std::forward<F>(callback) };

	// register new handler
	ret = sigaction(s, &sig, nullptr);
	if (ret == -1) {
		s_seat.m_handler = {};
		throw iptsd::utils::cerror("Failed to register signal handler");
	}
}

template<int s>
void SignalStub<s>::clear()
{
	int ret = sigaction(s, nullptr, nullptr);
	if (ret == -1)
		throw iptsd::utils::cerror("Failed to unregister signal handler");

	s_seat.m_handler = {};
}

} /* namespace detail */


template<int s, class F>
void signal(F&& callback) {
	if constexpr (std::is_same_v<F, std::nullptr_t>) {
		detail::SignalStub<s>::clear();
	} else {
		detail::SignalStub<s>::setup(std::forward<F>(callback));
	}
}

} /* namespace iptsd::utils */

#endif /* _IPTSD_COMMON_UTILS_SIGNAL_HPP_ */
