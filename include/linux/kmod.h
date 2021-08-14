/* $Id: kmod.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_KMOD_H
#define _LINUX_KMOD_H
#include <asm/errno.h>
static inline int request_module(const char *name, ...) { return -ENOSYS; }

#endif /* _LINUX_KMOD_H */
