/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef __SOUND_INITVAL_H
#define __SOUND_INITVAL_H

/*
 *  Init values for soundcard modules
 *  Copyright (c) by Jaroslav Kysela <perex@perex.cz>
 */

#ifndef MODULE_GENERIC_STRING
#ifdef MODULE
#define MODULE_GENERIC_STRING(name, string) \
static const char __module_generic_string_##name [] \
  __attribute__ ((section(".modstring"))) = #name "=" string;
#else
#define MODULE_GENERIC_STRING(name, string)
#endif
#endif

#ifdef TARGET_OS2
#define MODULE_CLASSES(val) 
#define MODULE_DEVICES(val) 
#define MODULE_PARM_SYNTAX(id, val) 
#else
#define MODULE_CLASSES(val) MODULE_GENERIC_STRING(info_classes, val)
#define MODULE_DEVICES(val) MODULE_GENERIC_STRING(info_devices, val)
#define MODULE_PARM_SYNTAX(id, val) MODULE_GENERIC_STRING(info_parm_##id, val)

#define SNDRV_MODULE_TYPE_int   "i"
#define SNDRV_MODULE_TYPE_bool  "i"
#define SNDRV_MODULE_TYPE_uint  "i"
#define SNDRV_MODULE_TYPE_charp "s"
#define SNDRV_MODULE_TYPE_long  "l"
#define module_param_array(name, type, nump, perm) \
        MODULE_PARM(name, "1-" __MODULE_STRING(SNDRV_CARDS) SNDRV_MODULE_TYPE_##type)
#define module_param(name, type, perm) \
        MODULE_PARM(name, SNDRV_MODULE_TYPE_##type)

#endif

#define SNDRV_AUTO_PORT		0xffff
#define SNDRV_AUTO_IRQ		0xffff
#define SNDRV_AUTO_DMA		0xffff
#define SNDRV_AUTO_DMA_SIZE	(0x7fffffff)

#define SNDRV_DEFAULT_IDX1	(-1)
#define SNDRV_DEFAULT_STR1	NULL
#define SNDRV_DEFAULT_ENABLE1	1
#define SNDRV_DEFAULT_PORT1	SNDRV_AUTO_PORT
#define SNDRV_DEFAULT_IRQ1	SNDRV_AUTO_IRQ
#define SNDRV_DEFAULT_DMA1	SNDRV_AUTO_DMA
#define SNDRV_DEFAULT_DMA_SIZE1	SNDRV_AUTO_DMA_SIZE
#define SNDRV_DEFAULT_PTR1	SNDRV_DEFAULT_STR1

#ifdef TARGET_OS2
#define REPEAT_SNDRV(a)         a,a,a,a,a,a,a,a
#define SNDRV_DEFAULT_IDX	{ REPEAT_SNDRV(-1) }
#define SNDRV_DEFAULT_STR	{ REPEAT_SNDRV(NULL) }
#define SNDRV_DEFAULT_ENABLE	{ 1,1,1,1,1,1,1,1 }
#define SNDRV_DEFAULT_ENABLE_PNP	SNDRV_DEFAULT_ENABLE
#define SNDRV_DEFAULT_PORT	{ REPEAT_SNDRV(-1) }
#define SNDRV_DEFAULT_IRQ	{ REPEAT_SNDRV(SNDRV_AUTO_IRQ) }
#define SNDRV_DEFAULT_DMA	{ REPEAT_SNDRV(SNDRV_AUTO_DMA) }
#define SNDRV_DEFAULT_DMA_SIZE	{ REPEAT_SNDRV(SNDRV_AUTO_DMA_SIZE) }
#define SNDRV_DEFAULT_PTR	SNDRV_DEFAULT_STR
#define SNDDRV_DEFAULT_PCM_DEVS       {REPEAT_SNDRV(1)};
#define SNDDRV_DEFAULT_PCM_SUBSTREAMS {REPEAT_SNDRV(8)};
#define SNDDRV_DEFAULT_MIDI_DEVS      {REPEAT_SNDRV(4)};
#else
#define SNDRV_DEFAULT_IDX	{ [0 ... (SNDRV_CARDS-1)] = -1 }
#define SNDRV_DEFAULT_STR	{ [0 ... (SNDRV_CARDS-1)] = NULL }
#define SNDRV_DEFAULT_ENABLE	{ 1, [1 ... (SNDRV_CARDS-1)] = 0 }
#define SNDRV_DEFAULT_ENABLE_PNP { [0 ... (SNDRV_CARDS-1)] = 1 }
#define SNDRV_DEFAULT_PORT	{ SNDRV_AUTO_PORT, [1 ... (SNDRV_CARDS-1)] = -1 }
#define SNDRV_DEFAULT_IRQ	{ [0 ... (SNDRV_CARDS-1)] = SNDRV_AUTO_IRQ }
#define SNDRV_DEFAULT_DMA	{ [0 ... (SNDRV_CARDS-1)] = SNDRV_AUTO_DMA }
#define SNDRV_DEFAULT_DMA_SIZE	{ [0 ... (SNDRV_CARDS-1)] = SNDRV_AUTO_DMA_SIZE }
#define SNDRV_DEFAULT_PTR	SNDRV_DEFAULT_STR
#endif

#ifdef CONFIG_PNP
#define SNDRV_DEFAULT_ENABLE_ISAPNP SNDRV_DEFAULT_ENABLE_PNP
#else
#define SNDRV_DEFAULT_ENABLE_ISAPNP SNDRV_DEFAULT_ENABLE
#endif

#define SNDRV_BOOLEAN_TRUE_DESC	"allows:{{0,Disabled},{1,Enabled}},default:1,dialog:check"
#define SNDRV_BOOLEAN_FALSE_DESC "allows:{{0,Disabled},{1,Enabled}},default:0,dialog:check"

#define SNDRV_ENABLED		"enable:(snd_enable)"

#define SNDRV_INDEX_DESC	SNDRV_ENABLED ",allows:{{0,7}},unique,skill:required,dialog:list"
#define SNDRV_ID_DESC		SNDRV_ENABLED ",unique"
#define SNDRV_ENABLE_DESC	SNDRV_BOOLEAN_FALSE_DESC
#define SNDRV_ISAPNP_DESC	SNDRV_ENABLED "," SNDRV_BOOLEAN_TRUE_DESC
#define SNDRV_DMA8_DESC		SNDRV_ENABLED ",allows:{{0,1},{3}},dialog:list"
#define SNDRV_DMA16_DESC	SNDRV_ENABLED ",allows:{{5,7}},dialog:list"
#define SNDRV_DMA_DESC		SNDRV_ENABLED ",allows:{{0,1},{3},{5,7}},dialog:list"
#define SNDRV_IRQ_DESC		SNDRV_ENABLED ",allows:{{5},{7},{9},{10,12},{14,15}},dialog:list"
#define SNDRV_DMA_SIZE_DESC	SNDRV_ENABLED ",allows:{{4,128}},default:64,skill:advanced"
#define SNDRV_DMA8_SIZE_DESC	SNDRV_ENABLED ",allows:{{4, 64}},default:64,skill:advanced"
#define SNDRV_DMA16_SIZE_DESC	SNDRV_ENABLED ",allows:{{4,128}},default:64,skill:advanced"
#define SNDRV_PORT12_DESC	SNDRV_ENABLED ",allows:{{0,0x3fff}},base:16"
#define SNDRV_PORT_DESC		SNDRV_ENABLED ",allows:{{0,0xffff}},base:16"

#ifdef SNDRV_LEGACY_FIND_FREE_IOPORT
static long snd_legacy_find_free_ioport(long *port_table, long size)
{
	while (*port_table != -1) {
		if (request_region(*port_table, size, "ALSA test")) {
			release_region(*port_table, size);
			return *port_table;
		}
		port_table++;
	}
	return -1;
}
#endif

#ifdef SNDRV_LEGACY_FIND_FREE_IRQ
#include <linux/interrupt.h>

static irqreturn_t snd_legacy_empty_irq_handler(int irq, void *dev_id)
{
	return IRQ_HANDLED;
}

static int snd_legacy_find_free_irq(int *irq_table)
{
	while (*irq_table != -1) {
		if (!request_irq(*irq_table, snd_legacy_empty_irq_handler,
				 IRQF_PROBE_SHARED, "ALSA Test IRQ",
				 (void *) irq_table)) {
			free_irq(*irq_table, (void *) irq_table);
			return *irq_table;
		}
		irq_table++;
	}
	return -1;
}
#endif

#ifdef SNDRV_LEGACY_FIND_FREE_DMA
static int snd_legacy_find_free_dma(int *dma_table)
{
	while (*dma_table != -1) {
		if (!request_dma(*dma_table, "ALSA Test DMA")) {
			free_dma(*dma_table);
			return *dma_table;
		}
		dma_table++;
	}
	return -1;
}
#endif

#if defined(SNDRV_GET_ID) && !defined(MODULE)
#include <linux/ctype.h>
static int __init get_id(char **str, char **dst)
{
	char *s, *d;

	if (!(*str) || !(**str))
		return 0;
	for (s = *str; isalpha(*s) || isdigit(*s) || *s == '_'; s++);
	if (s != *str) {
		*dst = (char *)kmalloc(s - *str, GFP_KERNEL);
		if ((d = *dst) != NULL) {
			s = *str;
			while (isalpha(*s) || isdigit(*s) || *s == '_')
				*d++ = *s++;
		}
	}
	*str = s;
	if (*s == ',') {
		(*str)++;
		return 2;
	}
	return 1;
}
#endif

#endif				/* __INITVAL_H */
