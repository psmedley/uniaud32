/* $Id: latency.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_LATENCY_H
#define _LINUX_LATENCY_H
static inline void set_acceptable_latency(char *identifier, int usecs) {}
static inline void modify_acceptable_latency(char *identifier, int usecs) {}
static inline void remove_acceptable_latency(char *identifier) {}
static inline void synchronize_acceptable_latency(void) {}
static inline int system_latency_constraint(void) {return 0;}

static inline int register_latency_notifier(struct notifier_block * nb) {return 0;}
static inline int unregister_latency_notifier(struct notifier_block * nb) {return 0;}

#define INFINITE_LATENCY 1000000

#endif /* _LINUX_LATENCY_H */
