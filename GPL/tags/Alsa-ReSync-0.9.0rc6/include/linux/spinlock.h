/* $Id: spinlock.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef __LINUX_SPINLOCK_H
#define __LINUX_SPINLOCK_H

#ifdef __SMP__
#include <asm/spinlock.h>

#else /* !SMP */

#define DEBUG_SPINLOCKS	0	/* 0 == no debugging, 1 == maintain lock state, 2 == full debug */

/*
 * Your basic spinlocks, allowing only a single CPU anywhere
 *
 * Gcc-2.7.x has a nasty bug with empty initializers.
 */
typedef unsigned long spinlock_t;

#define SPIN_LOCK_UNLOCKED  0

void spin_lock_init(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_lock_flag(spinlock_t *lock, unsigned long *flag);
int  spin_trylock(spinlock_t *lock);
void spin_unlock_wait(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

/*
 * Read-write spinlocks, allowing multiple readers
 * but only one writer.
 *
 * NOTE! it is quite common to have readers in interrupts
 * but no interrupt writers. For those circumstances we
 * can "mix" irq-safe locks - any writer needs to get a
 * irq-safe write-lock, but readers can get non-irqsafe
 * read-locks.
 *
 * Gcc-2.7.x has a nasty bug with empty initializers.
 */
//  typedef struct { int gcc_is_buggy; } rwlock_t;
  typedef unsigned long rwlock_t;
  #define RW_LOCK_UNLOCKED 0 


/*
 * These are the generic versions of the spinlocks and read-write
 * locks..
 */

#define spin_lock_irqsave(lock, flags) 	spin_lock_flag(lock, (unsigned long *)&flags)
#define spin_lock_irq(lock)		spin_lock(lock)
#define spin_lock_bh(lock)		spin_lock(lock)

#define read_lock_irqsave(lock, flags) 	spin_lock_flag(lock, (unsigned long *)&flags)
#define read_lock_irq(lock)		spin_lock(lock)
#define read_lock_bh(lock)		spin_lock(lock)

#define write_lock_irqsave(lock, flags)	spin_lock_flag(lock, (unsigned long *)&flags)
#define write_lock_irq(lock)		spin_lock(lock)
#define write_lock_bh(lock)		spin_lock(lock)

#define spin_unlock_irqrestore(lock, flags) spin_unlock(lock)
#define spin_unlock_irq(lock)		spin_unlock(lock)
#define spin_unlock_bh(lock)		spin_unlock(lock)

#define read_unlock_irqrestore(lock, flags) spin_unlock(lock)
#define read_unlock_irq(lock)		spin_unlock(lock)
#define read_unlock_bh(lock)		spin_unlock(lock)

#define write_unlock_irqrestore(lock, flags) spin_unlock(lock)
#define write_unlock_irq(lock)		spin_unlock(lock)
#define write_unlock_bh(lock)		spin_unlock(lock)

#define read_lock(lock)			spin_lock(lock)
#define read_unlock(lock)		spin_unlock(lock)
#define write_lock(lock)		spin_lock(lock)
#define write_unlock(lock)		spin_unlock(lock)

#endif /* !SMP */
#endif /* __LINUX_SPINLOCK_H */
