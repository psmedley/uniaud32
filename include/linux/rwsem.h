/* $Id: rwsem.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_RWSEM_H
#define _LINUX_RWSEM_H
static inline int down_write_trylock(struct rw_semaphore *sem) {return 0;}
#define init_rwsem(x) init_MUTEX(x)
#endif /* _LINUX_RWSEM_H */
