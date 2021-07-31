/* $Id: smp_lock.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_SMP_LOCK_H
#define _LINUX_SMP_LOCK_H
#define lock_kernel()                           do { } while(0)
#define unlock_kernel()                         do { } while(0)
#endif /* _LINUX_SMP_LOCK_H */
