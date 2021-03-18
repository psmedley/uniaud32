/* $Id: errno.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_ERRNO_H
#define _LINUX_ERRNO_H

#include <asm/errno.h>

#ifdef __KERNEL__

/* Should never be seen by user programs */
#define ERESTARTSYS	512
#define ERESTARTNOINTR	513
#define ERESTARTNOHAND	514	/* restart if no handler.. */
#define ENOIOCTLCMD	515	/* No ioctl command */
#define ENOTSUPP	524	/* Operation is not supported */

#endif
#define	ENOENT		2		/* No such file or directory */
#define	EIO		 5	/* I/O error */
#define	EEXIST		17		/* File exists */
#define EINVAL		 22
#endif
