//
//  IPTSKenerlUserShared.h
//  SurfaceTouchScreen
//
//  Created by Xavier on 2022/6/11.
//  Copyright © 2022 Xia Shangning. All rights reserved.
//

#ifndef IPTSKenerlUserShared_h
#define IPTSKenerlUserShared_h

#include <IOKit/IOTypes.h>

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

#define IPTS_TOUCH_REPORT_ID       0x40
#define IPTS_STYLUS_REPORT_ID      0x50

struct PACKED IPTSStylusHIDReport {
    UInt8 in_range:1;
    UInt8 touch:1;
    UInt8 side_button:1;
    UInt8 inverted:1;
    UInt8 eraser:1;
    UInt8 reserved:3;
    UInt16 x;
    UInt16 y;
    UInt16 tip_pressure;
    UInt16 x_tilt;
    UInt16 y_tilt;
    UInt16 scan_time;
};

#define IPTS_TOUCH_SCREEN_FINGER_CNT    10

struct PACKED IPTSFingerReport {
    UInt8 touch:1;
    UInt8 contact_id:7;
    UInt16 x;
    UInt16 y;
};

struct PACKED IPTSTouchHIDReport {
    IPTSFingerReport fingers[IPTS_TOUCH_SCREEN_FINGER_CNT];
    UInt8 contact_num;
};

struct PACKED IPTSHIDReport {
    UInt8 report_id;
    union {
        IPTSTouchHIDReport  touch;
        IPTSStylusHIDReport stylus;
    } report;
};

struct PACKED IPTSMetadataSize {
    UInt32 rows;
    UInt32 columns;
    UInt32 width;
    UInt32 height;
};

#ifdef KERNEL
struct PACKED IPTSMetadataTransform {
    UInt32 xx;
    UInt32 yx;
    UInt32 tx;
    UInt32 xy;
    UInt32 yy;
    UInt32 ty;
};
#else
struct PACKED IPTSMetadataTransform {
    Float32 xx;
    Float32 yx;
    Float32 tx;
    Float32 xy;
    Float32 yy;
    Float32 ty;
};
#endif

struct PACKED IPTSDeviceMetaData {
    IPTSMetadataSize size;
    UInt8 unknown1;
    IPTSMetadataTransform transform;
    UInt32 unknown2[16];
};

struct PACKED IPTSDeviceInfo {
    UInt16 vendor_id;
    UInt16 product_id;
    UInt8  max_contacts;
    IPTSDeviceMetaData meta_data;
};

enum {
    kMethodGetDeviceInfo,
    kMethodReceiveInput,
    kMethodSendHIDReport,
    kMethodToggleProcessingStatus,
    
    kNumberOfMethods
};

#endif /* IPTSKenerlUserShared_h */
