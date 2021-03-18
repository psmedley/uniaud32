#ifndef _LINUX_FIRMWARE_H
#define _LINUX_FIRMWARE_H
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>

#define FIRMWARE_NAME_MAX 30 
struct firmware {
	size_t size;
	u8 *data;
};
struct device;
int request_firmware(const struct firmware **fw, const char *name,
		     struct device *device);
static inline int request_firmware_nowait(
	struct module *module, bool uevent,
	const char *name, struct device *device, gfp_t gfp, void *context,
	void (*cont)(const struct firmware *fw, void *context))
{
	return -EINVAL;
}

void release_firmware(const struct firmware *fw);
void register_firmware(const char *name, const u8 *data, size_t size);
#endif
