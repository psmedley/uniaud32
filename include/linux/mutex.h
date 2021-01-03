#ifndef __LINUX_MUTEX_H
#define __LINUX_MUTEX_H

#include <asm/semaphore.h>
#include <linux/list.h>

struct mutex {
	atomic_long_t		owner;
	spinlock_t		wait_lock;
#ifdef CONFIG_MUTEX_SPIN_ON_OWNER
	struct optimistic_spin_queue osq; /* Spinner MCS lock */
#endif
	struct list_head	wait_list;
#ifdef CONFIG_DEBUG_MUTEXES
	void			*magic;
#endif
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	struct lockdep_map	dep_map;
#endif
};

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
