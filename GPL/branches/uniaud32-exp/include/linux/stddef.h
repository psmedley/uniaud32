#ifndef _LINUX_STDDEF_H
#define _LINUX_STDDEF_H

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#undef offsetof
#define offsetof(__typ,__id) ((size_t)&(((__typ*)0)->__id))

#endif
