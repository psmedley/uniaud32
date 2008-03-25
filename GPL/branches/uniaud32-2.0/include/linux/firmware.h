#ifndef _LINUX_FIRMWARE_H
#define _LINUX_FIRMWARE_H
#define FIRMWARE_NAME_MAX 30

struct firmware {
	unsigned int size;
	unsigned char *data;
};

int snd_compat_request_firmware(const struct firmware **fw, const char *name);
void snd_compat_release_firmware(const struct firmware *fw);

#define request_firmware(fw, name, device) snd_compat_request_firmware(fw, name)
#define release_firmware(fw) snd_compat_release_firmware(fw)

#define NEEDS_COMPAT_FW_LOADER

#endif
