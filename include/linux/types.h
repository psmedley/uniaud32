#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

//#pragma off (unreferenced)

#include <limits.h>
#include <linux/posix_types.h>
#include <linux/compiler.h>
#include <asm/types.h>

#ifndef __KERNEL_STRICT_NAMES

typedef __kernel_fd_set		fd_set;
typedef __kernel_dev_t		dev_t;
typedef __kernel_ino_t		ino_t;
typedef __kernel_mode_t		mode_t;
typedef __kernel_nlink_t	nlink_t;
typedef __kernel_off_t		off_t;
typedef __kernel_pid_t		pid_t;
typedef __kernel_uid_t		uid_t;
typedef __kernel_gid_t		gid_t;
typedef __kernel_daddr_t	daddr_t;
typedef __kernel_key_t		key_t;
typedef __kernel_suseconds_t	suseconds_t;

typedef __kernel_loff_t		loff_t;

/*
 * The following typedefs are also protected by individual ifdefs for
 * historical reasons:
 */
#ifndef _SIZE_T
#define _SIZE_T
typedef __kernel_size_t		size_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef __kernel_ssize_t	ssize_t;
#endif

#ifndef _PTRDIFF_T
#define _PTRDIFF_T
typedef __kernel_ptrdiff_t	ptrdiff_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef __kernel_time_t		time_t;
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef __kernel_clock_t	clock_t;
#endif

#ifndef _CADDR_T
#define _CADDR_T
typedef __kernel_caddr_t	caddr_t;
#endif

/* bsd */
typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned int		u_int;
typedef unsigned long		u_long;

/* sysv */
typedef unsigned char		unchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

#define BITS_PER_LONG	32

typedef unsigned long           dma_addr_t;

#ifndef __BIT_TYPES_DEFINED__
#define __BIT_TYPES_DEFINED__

typedef		__u8		u_int8_t;
typedef		__s8		int8_t;
typedef		__u16		u_int16_t;
typedef		__s16		int16_t;
typedef		__u32		u_int32_t;
typedef		__s32		int32_t;

#endif /* !(__BIT_TYPES_DEFINED__) */

typedef		__u8		uint8_t;
typedef		__u16		uint16_t;
typedef		__u32		uint32_t;

//#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
typedef		__u64		uint64_t;
typedef		__u64		u_int64_t;
typedef		__s64		int64_t;
//#endif

#endif /* __KERNEL_STRICT_NAMES */

/*
 * Below are truly Linux-specific types that should never collide with
 * any application/library that wants linux/types.h.
 */

struct ustat {
	__kernel_daddr_t	f_tfree;
	__kernel_ino_t		f_tinode;
	char			f_fname[6];
	char			f_fpack[6];
};

#define __inline__ __inline

typedef unsigned __nocast gfp_t;

#include <string.h>
typedef __u32 __le32;
typedef __u16 __le16;
typedef unsigned int fmode_t;

typedef int bool;
#define false	0
#define true 1
typedef	unsigned int		__uintptr_t;
typedef	__uintptr_t		uintptr_t;
typedef __u16 __be16;
typedef __u32 __be32;
typedef unsigned __poll_t;
typedef u32 phys_addr_t;
typedef s64			int64_t;

#define DECLARE_BITMAP(name,bits) \
        unsigned long name[((bits)+BITS_PER_LONG-1)/BITS_PER_LONG]

#endif /* _LINUX_TYPES_H */
