#ifndef __LINUX_WORKQUEUE_H
#define __LINUX_WORKQUEUE_H

#include <linux/timer.h>
#include <sound/compat_22.h>
/* we know this is used below exactly once for at most one waiter */

struct work_struct {
        unsigned long pending;
        struct list_head entry;
        void (*func)(void *);
        void *data;
        void *wq_data;
        struct timer_list timer;
};

struct workqueue_struct {
	spinlock_t lock;
	const char *name;
	struct list_head worklist;
	int task_pid;
	struct task_struct *task;
	wait_queue_head_t more_work;
	wait_queue_head_t work_done;
	struct completion thread_exited;
};
struct delayed_work {
	struct work_struct work;
};

struct workqueue_struct *create_workqueue(const char *name);
void destroy_workqueue(struct workqueue_struct *wq);
int queue_work(struct workqueue_struct *wq, struct work_struct *work);
void flush_workqueue(struct workqueue_struct *wq);
int queue_delayed_work(struct workqueue_struct *wq, struct delayed_work *dwork, unsigned long delay);
int cancel_delayed_work(struct delayed_work *dwork);

#define INIT_WORK(_work, _func, _data)			\
	do {						\
		(_work)->func = _func;			\
		(_work)->data = _data;			\
		init_timer(&(_work)->timer);		\
	} while (0)
#define __WORK_INITIALIZER(n, f, d) {			\
		.func = (f),				\
		.data = (d),				\
	}
#define DECLARE_WORK(n, f, d)                           \
        struct work_struct n = __WORK_INITIALIZER(n, f, d)

/* redefine INIT_WORK() */
static inline void snd_INIT_WORK(struct work_struct *w, void (*f)(struct work_struct *))
{
	INIT_WORK(w, (void(*)(void*))(f), w);
}
#undef INIT_WORK
#define INIT_WORK(w,f) snd_INIT_WORK(w,f)
#define INIT_DELAYED_WORK(_work, _func)	INIT_WORK(&(_work)->work, _func)
#define work_pending(work) test_bit(0, &(work)->pending)
#define delayed_work_pending(w) work_pending(&(w)->work)
#define schedule_delayed_work(work, delay) queue_delayed_work(NULL, (work), (delay))

#define create_singlethread_workqueue(name) create_workqueue(name)
#define cancel_delayed_work_sync flush_delayed_work_sync
#endif /* __LINUX_WORKQUEUE_H */
