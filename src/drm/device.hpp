/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_DRM_DEVICE_HPP
#define IPTSD_DRM_DEVICE_HPP

#include <common/types.hpp>

namespace iptsd::drm {

class Device {
public:
	f32 width;
	f32 height;

public:
	Device();
};

} /* namespace iptsd::drm */

#endif /* IPTSD_DRM_DEVICE_HPP */
