#ifndef _LINUX_MOD_DEVICETABLE_H
#define _LINUX_MOD_DEVICETABLE_H

#include <linux/types.h>

struct hda_device_id {
	__u32 vendor_id;
	__u32 rev_id;
	__u8 api_version;
	const char *name;
	unsigned long driver_data;
};

#define ACPI_ID_LEN	9

#endif /* _LINUX_MOD_DEVICETABLE_H */
