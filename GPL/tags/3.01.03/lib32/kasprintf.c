/*
 *  linux/lib/kasprintf.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include <stdarg.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/kernel.h>

/* Simplified asprintf. */
char *kvasprintf(gfp_t gfp, const char *fmt, va_list ap)
{
	unsigned int first, second;
	char *p;
	va_list aq;

	va_copy(aq, ap);
	first = vsnprintf(NULL, 0, fmt, aq);
	va_end(aq);

	p = kmalloc_track_caller(first+1, gfp);
	if (!p)
		return NULL;

	second = vsnprintf(p, first+1, fmt, ap);

	return p;
}

char *kasprintf(gfp_t gfp, const char *fmt, ...)
{
	va_list ap;
	char *p;

	va_start(ap, fmt);
	p = kvasprintf(gfp, fmt, ap);
	va_end(ap);

	return p;
}
