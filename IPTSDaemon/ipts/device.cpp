//
//  device.cpp
//  IPTSDaemon
//
//  Created by Xiashangning on 2023/2/10.
//

#include "device.hpp"

#include <common/cerror.hpp>

namespace iptsd::ipts {

Device::Device() {
    connect_to_kernel();
}

Device::~Device() {
    disconnect_from_kernel();
}

void Device::reset() {
    disconnect_from_kernel();
    sleep(2);
    connect_to_kernel();
}

void Device::connect_to_kernel()
{
    io_iterator_t   iterator;
    kern_return_t ret = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IntelPreciseTouchStylusDriver"), &iterator);
    if (ret != KERN_SUCCESS)
        throw common::cerror("Failed to match services");
    
    service = IOIteratorNext(iterator);
    IOObjectRelease(iterator);
    if (service == IO_OBJECT_NULL)
        throw common::cerror("Could not find IntelPreciseTouchStylusDriver");
    
    ret = IOServiceOpen(service, mach_task_self(), 0, &connect);
    if (ret != kIOReturnSuccess)
        throw common::cerror("Failed to establish a connection to the driver");

    IPTSDeviceInfo info;
    size_t info_size = sizeof(IPTSDeviceInfo);
    ret = IOConnectCallStructMethod(connect, kMethodGetDeviceInfo, nullptr, 0, &info, &info_size);
    if (ret != kIOReturnSuccess)
        throw common::cerror("Failed to get IPTS device info from driver");
    vendor_id = info.vendor_id;
    product_id = info.product_id;
    if (info.meta_data.size.rows != -1)
        meta_data = info.meta_data;
    
    uint64_t size;
    ret = IOConnectMapMemory(connect, 0, mach_task_self(), &input_buffer, &size, kIOMapAnywhere | kIOMapInhibitCache);
    if (ret != kIOReturnSuccess)
        throw common::cerror("Failed to map input buffer into user space");
}

void Device::disconnect_from_kernel()
{
    IOConnectUnmapMemory(connect, 0, mach_task_self(), input_buffer);
    
    IOServiceClose(connect);
    IOObjectRelease(service);
}

gsl::span<u8> Device::read() {
    UInt64 input_size = 0;
    UInt32 cnt = 1;
    kern_return_t ret = IOConnectCallScalarMethod(connect, kMethodReceiveInput, nullptr, 0, &input_size, &cnt);
    should_reinit = input_size == -1;
    if (ret != kIOReturnSuccess || should_reinit) {
        throw common::cerror("Failed to receive input!");
    }
    
    return gsl::span<u8>(reinterpret_cast<u8 *>(input_buffer), input_size);
}

void Device::send_hid_report(IPTSHIDReport &report) {
    kern_return_t ret = IOConnectCallStructMethod(connect, kMethodSendHIDReport, &report, sizeof(IPTSHIDReport), nullptr, nullptr);
    if (ret != kIOReturnSuccess)
        throw common::cerror("Failed to send HID report!");
}

void Device::process_begin() {
    if (!processing) {
        processing = true;
        UInt64 status = 1;
        kern_return_t ret = IOConnectCallScalarMethod(connect, kMethodToggleProcessingStatus, &status, 1, nullptr, nullptr);
        if (ret != kIOReturnSuccess)
            throw common::cerror("Failed to acquire input lock!");
    }
}

void Device::process_end() {
    if (processing) {
        processing = false;
        UInt64 status = 0;
        kern_return_t ret = IOConnectCallScalarMethod(connect, kMethodToggleProcessingStatus, &status, 1, nullptr, nullptr);
        if (ret != kIOReturnSuccess)
            throw common::cerror("Failed to release input lock!");
    }
}

} // namespace iptsd::ipts
