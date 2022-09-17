#ifndef _LINUX_KTIME_H
#define _LINUX_KTIME_H

#include <linux/time.h>
#include <linux/jiffies.h>

/* Nanosecond scalar representation for kernel time values */
typedef s64	ktime_t;

#define ktime_get_ts(x) do_posix_clock_monotonic_gettime(x)
/* Map the ktime_t to timespec conversion to ns_to_timespec function */
#define ktime_to_timespec(kt)	ns_to_timespec((kt).tv64)
#define ktime_get_ts64 		ktime_get_ts
#define ktime_get_raw_ts64 	getrawmonotonic
#define ktime_get_real_ts64 	getnstimeofday

/* Map the ktime_t to timespec conversion to ns_to_timespec function */
#define ktime_to_timespec64(kt)		ns_to_timespec64((kt))
#include <linux/timekeeping.h>

#endif /* _LINUX_KTIME_H */
