#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

#include <dbgos2.h>

#include <asm/param.h>	/* for HZ */
#include <asm/atomic.h>	
#include <linux/pid.h>

#define MAX_SCHEDULE_TIMEOUT    INT_MAX

#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		4
#define TASK_STOPPED		8
#define TASK_SWAPPING		16
#define TASK_EXCLUSIVE		32

#define set_current_state(a)

struct task_struct {
/* these are hardcoded - don't touch */
	         long state;	/* -1 unrunnable, 0 runnable, >0 stopped */
	unsigned long flags;	/* per process flags, defined below */
	int sigpending;
        unsigned long pid;
	char comm[16];
/* open file information */
	struct files_struct *files;
};

#include <asm\current.h>
#include <linux\wait.h>

extern void __wake_up(wait_queue_head_t *q, unsigned int mode);
extern void sleep_on(wait_queue_head_t *q);
extern long sleep_on_timeout(wait_queue_head_t *q,
				      signed long timeout);
extern void interruptible_sleep_on(wait_queue_head_t *q);
extern long interruptible_sleep_on_timeout(wait_queue_head_t *q,
						    signed long timeout);
extern void wake_up_process(struct task_struct * tsk);

#define wake_up(x)			__wake_up((x),TASK_UNINTERRUPTIBLE | TASK_INTERRUPTIBLE)
#define wake_up_interruptible(x)	__wake_up((x),TASK_INTERRUPTIBLE)

void schedule(void);

#define cond_resched() \
        do { \
                if (1) { \
                        set_current_state(TASK_RUNNING); \
                        schedule(); \
                } \
        } while (0)

// 12 Jun 07 SHL Drop superfluous near
#if 0
extern int request_irq(unsigned int,
		       int (*handler)(int, void *, struct pt_regs *),
		       unsigned long, const char *, void *);
#endif
extern void free_irq(unsigned int, void *);
extern void eoi_irq(unsigned int);

extern unsigned long volatile jiffies;

extern signed long schedule_timeout(signed long timeout);

static inline int signal_pending(struct task_struct *p)
{
#ifdef DEBUG
	dprintf(("signal_pending always returns 0"));
#endif /* DEBUG */
	return 0;
}

#define flush_scheduled_work()

#define schedule_timeout_interruptible(x) \
	set_current_state(TASK_INTERRUPTIBLE); \
	schedule_timeout(x);

#define schedule_timeout_uninterruptible(x) \
	set_current_state(TASK_UNINTERRUPTIBLE); \
	schedule_timeout(x);
#define TASK_NORMAL		(TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE)

#endif /* _LINUX_SCHED_H */
