// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_CONTACTS_DETECTION_ALGORITHMS_ERRORS_HPP
#define IPTSD_CONTACTS_DETECTION_ALGORITHMS_ERRORS_HPP

#include <common/types.hpp>

#include <string>

namespace iptsd::contacts::detection {

enum class Error : u8 {
	InvalidNeutralMode,
	InvalidClusterOverlap,
	FailedToMergeClusters,
};

inline std::string format_as(Error err)
{
	switch (err) {
	case Error::InvalidNeutralMode:
		return "contacts: Invalid neutral mode!";
	case Error::InvalidClusterOverlap:
		return "contacts: Calculated invalid cluster overlap!";
	case Error::FailedToMergeClusters:
		return "contacts: Failed to merge overlapping clusters!";
	default:
		return "contacts: Invalid error code!";
	}
}

}; // namespace iptsd::contacts::detection

#endif // IPTSD_CONTACTS_DETECTION_ALGORITHMS_ERRORS_HPP
