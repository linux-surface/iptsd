// SPDX-License-Identifier: GPL-2.0-or-later

#include "utils.hpp"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <system_error>

void Utils::signal(int signum, void (*handler)(int))
{
	struct sigaction sig;

	memset(&sig, 0, sizeof(sig));
	sig.sa_handler = handler;

	int ret = sigaction(signum, &sig, nullptr);
	if (ret == -1)
		throw Utils::cerror("Failed to register signal handler");
}

std::system_error Utils::cerror(std::string msg)
{
	std::error_code code(errno, std::system_category());
	return std::system_error(code, msg);
}
