/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef IPTSD_HID_SHIM_HIDRD_H
#define IPTSD_HID_SHIM_HIDRD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define hidrd_item	    uint8_t
#define hidrd_item_main_tag int64_t
#define hidrd_usage	    int64_t
#define hidrd_usage_page    int64_t

size_t shim_hidrd_item_get_size(const uint8_t *item);
uint8_t shim_hidrd_item_report_id_get_value(const uint8_t *item);
int64_t shim_hidrd_item_basic_get_format(const uint8_t *item);
int64_t shim_hidrd_item_short_get_type(const uint8_t *item);
int64_t shim_hidrd_item_global_get_tag(const uint8_t *item);
int64_t shim_hidrd_item_main_get_tag(const uint8_t *item);
int64_t shim_hidrd_item_local_get_tag(const uint8_t *item);
int64_t shim_hidrd_item_usage_get_value(const uint8_t *item);
int64_t shim_hidrd_item_usage_page_get_value(const uint8_t *item);
int64_t shim_hidrd_item_report_count_get_value(const uint8_t *item);
int64_t shim_hidrd_item_report_size_get_value(const uint8_t *item);

int64_t shim_hidrd_item_basic_format_short();
int64_t shim_hidrd_item_short_type_global();
int64_t shim_hidrd_item_short_type_main();
int64_t shim_hidrd_item_global_tag_report_id();
int64_t shim_hidrd_item_main_tag_input();
int64_t shim_hidrd_item_main_tag_output();
int64_t shim_hidrd_item_main_tag_feature();
int64_t shim_hidrd_item_short_type_local();
int64_t shim_hidrd_item_local_tag_usage();
int64_t shim_hidrd_usage_page_max();
int64_t shim_hidrd_item_global_tag_usage_page();
int64_t shim_hidrd_item_global_tag_report_count();
int64_t shim_hidrd_item_global_tag_report_size();
int64_t shim_hidrd_usage_page_digitizer();

#define hidrd_item_get_size		  shim_hidrd_item_get_size
#define hidrd_item_report_id_get_value	  shim_hidrd_item_report_id_get_value
#define hidrd_item_basic_get_format	  shim_hidrd_item_basic_get_format
#define hidrd_item_short_get_type	  shim_hidrd_item_short_get_type
#define hidrd_item_global_get_tag	  shim_hidrd_item_global_get_tag
#define hidrd_item_main_get_tag		  shim_hidrd_item_main_get_tag
#define hidrd_item_local_get_tag	  shim_hidrd_item_local_get_tag
#define hidrd_item_usage_get_value	  shim_hidrd_item_usage_get_value
#define hidrd_item_usage_page_get_value	  shim_hidrd_item_usage_page_get_value
#define hidrd_item_report_count_get_value shim_hidrd_item_report_count_get_value
#define hidrd_item_report_size_get_value  shim_hidrd_item_report_size_get_value

#define HIDRD_ITEM_BASIC_FORMAT_SHORT	   shim_hidrd_item_basic_format_short()
#define HIDRD_ITEM_SHORT_TYPE_GLOBAL	   shim_hidrd_item_short_type_global()
#define HIDRD_ITEM_SHORT_TYPE_MAIN	   shim_hidrd_item_short_type_main()
#define HIDRD_ITEM_GLOBAL_TAG_REPORT_ID	   shim_hidrd_item_global_tag_report_id()
#define HIDRD_ITEM_MAIN_TAG_INPUT	   shim_hidrd_item_main_tag_input()
#define HIDRD_ITEM_MAIN_TAG_OUTPUT	   shim_hidrd_item_main_tag_output()
#define HIDRD_ITEM_MAIN_TAG_FEATURE	   shim_hidrd_item_main_tag_feature()
#define HIDRD_ITEM_SHORT_TYPE_LOCAL	   shim_hidrd_item_short_type_local()
#define HIDRD_ITEM_LOCAL_TAG_USAGE	   shim_hidrd_item_local_tag_usage()
#define HIDRD_USAGE_PAGE_MAX		   shim_hidrd_usage_page_max()
#define HIDRD_ITEM_GLOBAL_TAG_USAGE_PAGE   shim_hidrd_item_global_tag_usage_page()
#define HIDRD_ITEM_GLOBAL_TAG_REPORT_COUNT shim_hidrd_item_global_tag_report_count()
#define HIDRD_ITEM_GLOBAL_TAG_REPORT_SIZE  shim_hidrd_item_global_tag_report_size()
#define HIDRD_USAGE_PAGE_DIGITIZER	   shim_hidrd_usage_page_digitizer()

#ifdef __cplusplus
}
#endif

#endif /* IPTSD_HID_SHIM_HIDRD_H */
