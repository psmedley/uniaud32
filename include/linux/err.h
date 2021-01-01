/* $Id: err.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_ERR_H
#define _LINUX_ERR_H
#define IS_ERR_VALUE(x) ((x) > (unsigned long)-1000L)

static inline void *ERR_PTR(long error)
{
	return (void *) error;
}

static inline long PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

static inline long IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

static inline int PTR_RET(const void *ptr)
{
	if (IS_ERR(ptr))
		return PTR_ERR(ptr);
	else
		return 0;
}

#define PTR_ERR_OR_ZERO(p) PTR_RET(p)
#endif /* _LINUX_ERR_H */
