/* $Id: rwsem.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_RWSEM_H
#define _LINUX_RWSEM_H
/* rw_semaphore - replaced with mutex */
#define rw_semaphore semaphore
#define init_rwsem(x) init_MUTEX(x)
#define DECLARE_RWSEM(x) DECLARE_MUTEX(x)
#define down_read(x) down(x)
#define down_write(x) down(x)
#define up_read(x) up(x)
#define up_write(x) up(x)

static inline int down_write_trylock(struct rw_semaphore *sem) {return 0;}
#endif /* _LINUX_RWSEM_H */
