#ifndef __LINUX_LOCKDEP_H
#define __LINUX_LOCKDEP_H
/*
 * lockdep macros
 */
#define lockdep_set_class(lock, key)		do { (void)(key); } while (0)
#define down_read_nested(sem, subclass)		down_read(sem)
#define down_write_nested(sem, subclass)	down_write(sem)
#define down_read_non_owner(sem)		down_read(sem)
#define up_read_non_owner(sem)			up_read(sem)
#define spin_lock_nested(lock, x)		spin_lock(lock)
#define spin_lock_irqsave_nested(lock, f, x)	spin_lock_irqsave(lock, f)

#endif /* __LINUX_LOCKDEP_H */
