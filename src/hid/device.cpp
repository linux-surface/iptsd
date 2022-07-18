// SPDX-License-Identifier: GPL-2.0-or-later

#include "device.hpp"

#include "descriptor.hpp"

#include <common/cerror.hpp>
#include <common/cwrap.hpp>

#include <gsl/gsl>
#include <linux/hidraw.h>
#include <string>
#include <sys/types.h>
#include <vector>

namespace iptsd::hid {

Device::Device(const std::string &path)
{
	int ret = common::open(path, O_RDWR);
	if (ret == -1)
		throw common::cerror("Failed to open hidraw device");

	this->device = ret;

	ret = common::ioctl(this->device, HIDIOCGRAWINFO, &this->devinfo);
	if (ret == -1)
		throw common::cerror("Failed to read HID device info");

	u32 desc_size = 0;

	ret = common::ioctl(this->device, HIDIOCGRDESCSIZE, &desc_size);
	if (ret == -1)
		throw common::cerror("Failed to read HID descriptor size");

	struct hidraw_report_descriptor hidraw_desc = {};
	hidraw_desc.size = desc_size;

	ret = common::ioctl(this->device, HIDIOCGRDESC, &hidraw_desc);
	if (ret == -1)
		throw common::cerror("Failed to read HID descriptor");

	this->desc.load(gsl::span<u8>(&hidraw_desc.value[0], desc_size));
}

i16 Device::product()
{
	return this->devinfo.product;
}

i16 Device::vendor()
{
	return this->devinfo.vendor;
}

const Descriptor &Device::descriptor()
{
	return this->desc;
}

ssize_t Device::read(gsl::span<u8> buffer)
{
	ssize_t ret = common::read(this->device, buffer);
	if (ret == -1)
		throw common::cerror("Failed to read from HID device");

	return ret;
}

void Device::get_feature(gsl::span<u8> report)
{
	int ret = common::ioctl(this->device, HIDIOCGFEATURE(report.size()), report.data());
	if (ret == -1)
		throw common::cerror("Failed to get feature");
}

void Device::set_feature(gsl::span<u8> report)
{
	int ret = common::ioctl(this->device, HIDIOCSFEATURE(report.size()), report.data());
	if (ret == -1)
		throw common::cerror("Failed to set feature");
}

} // namespace iptsd::hid
