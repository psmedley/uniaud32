#ifndef _LINUX_COMPLETION_H
#define _LINUX_COMPLETION_H

#include <linux/wait.h>
#include <sound/compat_22.h>
/*
 * struct completion - structure used to maintain state for a "completion"
 *
 * This is the opaque structure used to maintain the state for a "completion".
 * Completions currently use a FIFO to queue threads that have to wait for
 * the "completion" event.
 *
 * See also:  complete(), wait_for_completion() (and friends _timeout,
 * _interruptible, _interruptible_timeout, and _killable), init_completion(),
 * reinit_completion(), and macros DECLARE_COMPLETION(),
 * DECLARE_COMPLETION_ONSTACK().
 */
struct completion {
	unsigned int done;
	wait_queue_head_t wait;
};

#define DECLARE_COMPLETION_ONSTACK(work) 	struct completion work;
/**
 * init_completion - Initialize a dynamically allocated completion
 * @x:  completion structure that is to be initialized
 *
 * This inline function will initialize a dynamically created completion
 * structure.
 */
static inline void init_completion(struct completion *x)
{
	x->done = 0;
	init_waitqueue_head(&x->wait);
}

extern void complete(struct completion *);
extern void complete_all(struct completion *);
extern void wait_for_completion(struct completion *x);
extern bool try_wait_for_completion(struct completion *x);
#endif /* _LINUX_COMPLETION_H */
