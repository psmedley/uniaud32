#ifndef _LINUX_ASYNC_H
#define _LINUX_ASYNC_H

#include <linux/list.h>
#include <linux/types.h>
typedef u64 async_cookie_t;
typedef void (*async_func_t) (void *data, async_cookie_t cookie);
struct async_domain {
	struct list_head pending;
	unsigned registered:1;
};

/*
 * domain is free to go out of scope as soon as all pending work is
 * complete, this domain does not participate in async_synchronize_full
 */
#define ASYNC_DOMAIN_EXCLUSIVE(_name) \
	struct async_domain _name = {  \
				      .registered = 0 }

#define async_synchronize_full_domain(domain) 

#define async_schedule_domain(func, data, domain)
#endif /* _LINUX_ASYNC_H */
