#ifndef __LINUX_WORKQUEUE_H
#define __LINUX_WORKQUEUE_H
struct work_struct {
        unsigned long pending;
        struct list_head entry;
        void (*func)(void *);
        void *data;
        void *wq_data;
        struct timer_list timer;
};

struct delayed_work {
	struct work_struct work;
};

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
#define create_singlethread_workqueue(name) create_workqueue(name)
#endif /* __LINUX_WORKQUEUE_H */
