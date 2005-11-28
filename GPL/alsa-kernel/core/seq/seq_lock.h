#ifndef __SND_SEQ_LOCK_H
#define __SND_SEQ_LOCK_H

#if defined(__SMP__) || defined(CONFIG_SND_DEBUG)
typedef atomic_t snd_use_lock_t;

#define snd_use_lock_init(lockp) atomic_set(lockp, 0)
#define snd_use_lock_use(lockp) atomic_inc(lockp)
#define snd_use_lock_free(lockp) atomic_dec(lockp)
#define snd_use_lock_sync(lockp) do {\
	int max_count = 5 * HZ;\
	if (atomic_read(lockp) < 0) {\
		printk("seq_lock: lock trouble [counter = %d] in %s:%d\n", atomic_read(lockp), __BASE_FILE__, __LINE__);\
	}\
	while (atomic_read(lockp) > 0) {\
		if (max_count == 0) {\
			snd_printk("seq_lock: timeout [%d left] in %s:%d\n", atomic_read(lockp), __BASE_FILE__, __LINE__);\
			break;\
		}\
		set_current_state(TASK_UNINTERRUPTIBLE);\
		schedule_timeout(1);\
		max_count--;\
	}\
} while (0)

#else
typedef spinlock_t snd_use_lock_t;
#define snd_use_lock_init(lockp) /**/
#define snd_use_lock_use(lockp) /**/
#define snd_use_lock_free(lockp) /**/
#define snd_use_lock_sync(lockp) /**/
#endif

extern void snd_seq_sleep_in_lock(wait_queue_head_t *p, spinlock_t *lock);
extern long snd_seq_sleep_timeout_in_lock(wait_queue_head_t *p, spinlock_t *lock, long timeout);

#endif /* __SND_SEQ_LOCK_H */

