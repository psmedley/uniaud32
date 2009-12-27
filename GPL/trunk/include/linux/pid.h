#ifndef _LINUX_PID_H
#define _LINUX_PID_H

/* use nr as pointer */
struct pid;
#define get_pid(p) (p)
#define put_pid(p)
#define task_pid(t) ((struct pid *)((t)->pid))
#define pid_vnr(p) ((pid_t)(p))


#endif /* _LINUX_PID_H */
