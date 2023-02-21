//
//  device.hpp
//  IPTSDaemon
//
//  Created by Xiashangning on 2023/2/10.
//

#ifndef device_hpp
#define device_hpp

#include "IPTSKenerlUserShared.h"
#include <common/types.hpp>

#include <gsl/gsl>
#include <optional>
#include <IOKit/IOKitLib.h>

namespace iptsd::ipts {

class Device {
public:
    Device();
    ~Device();
    
    void reset();
    
    gsl::span<u8> read();
    
    void send_hid_report(IPTSHIDReport &report);
    
    void process_begin();
    void process_end();
    
    IOVirtualAddress input_buffer;
    bool should_reinit {false};
    
    i16 vendor_id  {0};
    i16 product_id {0};
    std::optional<IPTSDeviceMetaData> meta_data {std::nullopt};
private:    
    io_connect_t connect;
    io_service_t service;
    bool processing {false};
    
    void connect_to_kernel();
    void disconnect_from_kernel();
};

} // namespace iptsd::ipts

#endif /* device_hpp */
