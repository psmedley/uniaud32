#ifndef _LINUX_STRING_H
#define _LINUX_STRING_H
#if 0
char *strstr1 (const char *string1, const char *string2);

char *strncpy (char *string1, const char *string2, size_t count);

int   strcmp (const char *string1, const char *string2);

int   strncmp (const char *string1, const char *string2, size_t count);
#endif

char *kstrdup(const char *s, unsigned int gfp_flags);
void *memdup_user(const void __user *src, size_t len);
#define strnlen strnlen_s
_WCRTLINK extern size_t  strnlen_s( const char *__s, size_t __maxsize );
#endif

