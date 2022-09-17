#ifndef _LINUX_STRING_H
#define _LINUX_STRING_H

#include <linux/slab.h>

#if 0
char *strstr1 (const char *string1, const char *string2);

char *strncpy (char *string1, const char *string2, size_t count);

int   strcmp (const char *string1, const char *string2);

int   strncmp (const char *string1, const char *string2, size_t count);
#endif

char *kstrdup(const char *s, unsigned int gfp_flags);
_WCRTLINK extern size_t  strnlen_s( const char *__s, size_t __maxsize );
#define strnlen strnlen_s
extern void *memdup_user(const void __user *, size_t);

static inline void *kmemdup(const void *src, size_t len, gfp_t gfp)
{
	void *dst = kmalloc(len, gfp);
	if (!dst)
		return NULL;
	memcpy(dst, src, len);
	return dst;
}

#define vmemdup_user memdup_user
#define scnprintf snprintf
ssize_t strscpy(char *dest, const char *src, size_t count);

#endif

