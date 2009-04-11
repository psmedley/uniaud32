/* $Id: wait.h,v 1.1.1.1 2003/07/02 13:57:02 eleph Exp $ */

#ifndef _LINUX_WAIT_H
#define _LINUX_WAIT_H

#define WNOHANG		0x00000001
#define WUNTRACED	0x00000002

#define __WCLONE	0x80000000

#ifdef __KERNEL__

#include <linux/spinlock.h>

#include <asm/page.h>

/*
 * Temporary debugging help until all code is converted to the new
 * waitqueue usage.
 */

struct __wait_queue {
	unsigned int compiler_warning;
	struct task_struct * task;
	void * task_list;
#if WAITQUEUE_DEBUG
	long __magic;
	long __waker;
#endif
};
typedef struct __wait_queue wait_queue_t;

/*
 * 'dual' spinlock architecture. Can be switched between spinlock_t and
 * rwlock_t locks via changing this define. Since waitqueues are quite
 * decoupled in the new architecture, lightweight 'simple' spinlocks give
 * us slightly better latencies and smaller waitqueue structure size.
 */
#define USE_RW_WAIT_QUEUE_SPINLOCK 0

#if USE_RW_WAIT_QUEUE_SPINLOCK
# define wq_lock_t rwlock_t
# define WAITQUEUE_RW_LOCK_UNLOCKED RW_LOCK_UNLOCKED

# define wq_read_lock read_lock
# define wq_read_lock_irqsave read_lock_irqsave
# define wq_read_unlock_irqrestore read_unlock_irqrestore
# define wq_read_unlock read_unlock
# define wq_write_lock_irq write_lock_irq
# define wq_write_lock_irqsave write_lock_irqsave
# define wq_write_unlock_irqrestore write_unlock_irqrestore
# define wq_write_unlock write_unlock
#else
# define wq_lock_t spinlock_t
# define WAITQUEUE_RW_LOCK_UNLOCKED SPIN_LOCK_UNLOCKED

# define wq_read_lock spin_lock
# define wq_read_lock_irqsave spin_lock_irqsave
# define wq_read_unlock spin_unlock
# define wq_read_unlock_irqrestore spin_unlock_irqrestore
# define wq_write_lock_irq spin_lock_irq
# define wq_write_lock_irqsave spin_lock_irqsave
# define wq_write_unlock_irqrestore spin_unlock_irqrestore
# define wq_write_unlock spin_unlock
#endif

struct __wait_queue_head {
	void * lock;
	void * task_list;
#if WAITQUEUE_DEBUG
	long __magic;
	long __creator;
#endif
};
typedef struct __wait_queue_head wait_queue_head_t;

#if WAITQUEUE_DEBUG
# define __WAITQUEUE_DEBUG_INIT(name) \
		, (long)&(name).__magic, 0
# define __WAITQUEUE_HEAD_DEBUG_INIT(name) \
		, (long)&(name).__magic, (long)&(name).__magic
#else
# define __WAITQUEUE_DEBUG_INIT(name)
# define __WAITQUEUE_HEAD_DEBUG_INIT(name)
#endif

#define __WAITQUEUE_INITIALIZER(name,task) \
	{ 0x1234567, NULL, NULL __WAITQUEUE_DEBUG_INIT(name)}
#define DECLARE_WAITQUEUE(name,task) \
	wait_queue_t name = __WAITQUEUE_INITIALIZER(name,task)

#define __WAIT_QUEUE_HEAD_INITIALIZER(name) \
{ WAITQUEUE_RW_LOCK_UNLOCKED, { &(name).task_list, &(name).task_list } \
		__WAITQUEUE_HEAD_DEBUG_INIT(name)}

#define DECLARE_WAIT_QUEUE_HEAD(name) \
	wait_queue_head_t name = __WAIT_QUEUE_HEAD_INITIALIZER(name)

void init_waitqueue_head(wait_queue_head_t *q);

void init_waitqueue_entry(wait_queue_t *q, struct task_struct *p);

int waitqueue_active(wait_queue_head_t *q);

void __add_wait_queue(wait_queue_head_t *head, wait_queue_t *new);

/*
 * Used for wake-one threads:
 */
void __add_wait_queue_tail(wait_queue_head_t *head, wait_queue_t *new);
void __remove_wait_queue(wait_queue_head_t *head, wait_queue_t *old);

#endif /* __KERNEL__ */

#endif
