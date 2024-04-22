// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef IPTSD_APPS_CHECK_DEVICE_CHECK_HPP
#define IPTSD_APPS_CHECK_DEVICE_CHECK_HPP

#include <common/error.hpp>
#include <common/types.hpp>
#include <core/generic/application.hpp>
#include <core/generic/config.hpp>
#include <core/generic/device.hpp>
#include <ipts/device.hpp>

#include <gsl/gsl>

namespace iptsd::apps::check {
namespace impl {

enum class CheckError : u8 {
	WrongDeviceType,
};

inline std::string format_as(CheckError err)
{
	switch (err) {
	case CheckError::WrongDeviceType:
		return "apps: check-device: The device has the wrong type!";
	default:
		return "Invalid error code!";
	}
}

} // namespace impl

class Check : public core::Application {
public:
	using Error = impl::CheckError;

public:
	Check(const core::Config &config,
	      const core::DeviceInfo &info,
	      std::optional<ipts::Device::Type> type)
		: core::Application(config, info)
	{
		if (type.has_value() && info.type != type.value())
			throw common::Error<Error::WrongDeviceType> {};
	}
};

} // namespace iptsd::apps::check

#endif // IPTSD_APPS_CHECK_DEVICE_CHECK_HPP
