#ifndef __LINUX_MUTEX_H
#define __LINUX_MUTEX_H

#include <asm/semaphore.h>

#define mutex semaphore
#define DEFINE_MUTEX(x)		DECLARE_MUTEX(x)
#define mutex_init(x)		init_MUTEX(x)
#define mutex_destroy(x)
#define mutex_lock(x)		down(x)
#define mutex_lock_interruptible(x) down_interruptible(x)
#define mutex_unlock(x)		up(x)
#define mutex_lock_nested(lock, subclass) mutex_lock(lock)
static inline int mutex_trylock(struct mutex *lock) {return -1; }
#endif
