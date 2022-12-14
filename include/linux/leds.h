#ifndef _LINUX_LEDS_H
#define _LINUX_LEDS_H

#include <linux/workqueue.h>

/* This is obsolete/useless. We now support variable maximum brightness. */
enum led_brightness {
	LED_OFF		= 0,
	LED_ON		= 1,
	LED_HALF	= 127,
	LED_FULL	= 255,
};

struct led_classdev {
	const char		*name;
	enum led_brightness	 brightness;
	enum led_brightness	 max_brightness;
	int			 flags;
	struct device		*dev;
};

enum led_audio {
        LED_AUDIO_MUTE,         /* master mute LED */
        LED_AUDIO_MICMUTE,      /* mic mute LED */
        NUM_AUDIO_LEDS
};
#endif /* _LINUX_LEDS_H */
