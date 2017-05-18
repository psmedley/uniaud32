#ifndef _LINUX_DMI_H
#define _LINUX_DMI_H
/* dmi wrappers */
#define dmi_find_device(a, b, c)	NULL
#define DMI_DEV_TYPE_OEM_STRING		-2

struct dmi_device {
      struct list_head list;
      int type;
      const char *name;
      void *device_data;      /* Type specific data */
};

#endif /* _LINUX_DMI_H */

