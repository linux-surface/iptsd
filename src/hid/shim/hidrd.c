// SPDX-License-Identifier: GPL-2.0-or-later

#include <hidrd/item/any.h>
#include <stddef.h>
#include <stdint.h>

size_t shim_hidrd_item_get_size(const uint8_t *item)
{
	return hidrd_item_get_size((const hidrd_item *)item);
}

uint8_t shim_hidrd_item_report_id_get_value(const uint8_t *item)
{
	return hidrd_item_report_id_get_value((const hidrd_item *)item);
}

int64_t shim_hidrd_item_basic_get_format(const uint8_t *item)
{
	return hidrd_item_basic_get_format((const hidrd_item *)item);
}

int64_t shim_hidrd_item_short_get_type(const uint8_t *item)
{
	return hidrd_item_short_get_type((const hidrd_item *)item);
}

int64_t shim_hidrd_item_global_get_tag(const uint8_t *item)
{
	return hidrd_item_global_get_tag((const hidrd_item *)item);
}

int64_t shim_hidrd_item_main_get_tag(const uint8_t *item)
{
	return hidrd_item_main_get_tag((const hidrd_item *)item);
}

int64_t shim_hidrd_item_local_get_tag(const uint8_t *item)
{
	return hidrd_item_local_get_tag((const hidrd_item *)item);
}

int64_t shim_hidrd_item_usage_get_value(const uint8_t *item)
{
	return hidrd_item_usage_get_value((const hidrd_item *)item);
}

int64_t shim_hidrd_item_usage_page_get_value(const uint8_t *item)
{
	return hidrd_item_usage_page_get_value((const hidrd_item *)item);
}

int64_t shim_hidrd_item_report_count_get_value(const uint8_t *item)
{
	return hidrd_item_report_count_get_value((const hidrd_item *)item);
}

int64_t shim_hidrd_item_report_size_get_value(const uint8_t *item)
{
	return hidrd_item_report_size_get_value((const hidrd_item *)item);
}

int64_t shim_hidrd_item_basic_format_short()
{
	return HIDRD_ITEM_BASIC_FORMAT_SHORT;
}

int64_t shim_hidrd_item_short_type_global()
{
	return HIDRD_ITEM_SHORT_TYPE_GLOBAL;
}

int64_t shim_hidrd_item_short_type_main()
{
	return HIDRD_ITEM_SHORT_TYPE_MAIN;
}

int64_t shim_hidrd_item_global_tag_report_id()
{
	return HIDRD_ITEM_GLOBAL_TAG_REPORT_ID;
}

int64_t shim_hidrd_item_main_tag_input()
{
	return HIDRD_ITEM_MAIN_TAG_INPUT;
}

int64_t shim_hidrd_item_main_tag_output()
{
	return HIDRD_ITEM_MAIN_TAG_OUTPUT;
}

int64_t shim_hidrd_item_main_tag_feature()
{
	return HIDRD_ITEM_MAIN_TAG_FEATURE;
}

int64_t shim_hidrd_item_short_type_local()
{
	return HIDRD_ITEM_SHORT_TYPE_LOCAL;
}

int64_t shim_hidrd_item_local_tag_usage()
{
	return HIDRD_ITEM_LOCAL_TAG_USAGE;
}

int64_t shim_hidrd_usage_page_max()
{
	return HIDRD_USAGE_PAGE_MAX;
}

int64_t shim_hidrd_item_global_tag_usage_page()
{
	return HIDRD_ITEM_GLOBAL_TAG_USAGE_PAGE;
}

int64_t shim_hidrd_item_global_tag_report_count()
{
	return HIDRD_ITEM_GLOBAL_TAG_REPORT_COUNT;
}

int64_t shim_hidrd_item_global_tag_report_size()
{
	return HIDRD_ITEM_GLOBAL_TAG_REPORT_SIZE;
}

int64_t shim_hidrd_usage_page_digitizer()
{
	return HIDRD_USAGE_PAGE_DIGITIZER;
}
