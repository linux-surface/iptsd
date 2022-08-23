// SPDX-License-Identifier: GPL-2.0-or-later

#include "device.hpp"

#include "drm_mode.h"

#include <common/cerror.hpp>
#include <common/cwrap.hpp>

#include <filesystem>
#include <gsl/gsl>
#include <stdexcept>
#include <xf86drmMode.h>

namespace iptsd::drm {

Device::Device()
{
	int device = -1;
	drmModeConnector *connector = nullptr;

	if (!std::filesystem::exists("/dev/dri"))
		throw std::runtime_error {"/dev/dri does not exist!"};

	// Look in all DRI devices for the eDP connector (internal display)
	for (auto &p : std::filesystem::directory_iterator("/dev/dri")) {
		std::string basename = p.path().filename().string();

		if (basename.rfind("card", 0) != 0)
			continue;

		device = common::open(p.path(), O_RDWR);
		if (device == -1)
			throw common::cerror("Failed to open " + p.path().string());

		drmModeRes *res = drmModeGetResources(device);

		// Search for the eDP connector
		for (i32 i = 0; i < res->count_connectors; i++) {
			// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
			drmModeConnector *c = drmModeGetConnector(device, res->connectors[i]);

			if (c->connection == DRM_MODE_CONNECTED &&
			    c->connector_type == DRM_MODE_CONNECTOR_eDP) {
				connector = c;
				break;
			}

			drmModeFreeConnector(c);
		}

		drmModeFreeResources(res);

		if (connector)
			break;
	}

	if (device == -1 || !connector)
		throw std::runtime_error {"Could not find the eDP connector!"};

	this->width = gsl::narrow<f32>(connector->mmWidth) / 10;
	this->height = gsl::narrow<f32>(connector->mmHeight) / 10;

	drmModeFreeConnector(connector);
	close(device);
}

} // namespace iptsd::drm
