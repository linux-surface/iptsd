// SPDX-License-Identifier: GPL-2.0-or-later

#include "signal.hpp"

#include "cerror.hpp"

#include <csignal>
#include <cstring>

namespace iptsd::utils {

void signal(int signum, void (*handler)(int))
{
	struct sigaction sig;

	std::memset(&sig, 0, sizeof(sig));
	sig.sa_handler = handler;

	int ret = sigaction(signum, &sig, nullptr);
	if (ret == -1)
		throw iptsd::utils::cerror("Failed to register signal handler");
}

} /* namespace iptsd::utils */
