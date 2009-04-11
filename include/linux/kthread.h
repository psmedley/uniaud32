/* $Id: kthread.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_KTHREAD_H
#define _LINUX_KTHREAD_H
/* TODO: actually implement this ... */

#define kthread_create(fn, data, namefmt, ...) NULL
#define kthread_run(fn, data, namefmt, ...) NULL
#define kthread_stop(kt) 0
#define kthread_should_stop() 1

#endif /* _LINUX_KTHREAD_H */
