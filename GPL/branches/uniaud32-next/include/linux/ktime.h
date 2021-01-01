#ifndef _LINUX_KTIME_H
#define _LINUX_KTIME_H

#include <linux/time.h>
#include <linux/jiffies.h>

/*
 * ktime_t:
 *
 * A single 64-bit variable is used to store the hrtimers
 * internal representation of time values in scalar nanoseconds. The
 * design plays out best on 64-bit CPUs, where most conversions are
 * NOPs and most arithmetic ktime_t operations are plain arithmetic
 * operations.
 *
 */
union ktime {
	s64	tv64;
};

typedef union ktime ktime_t;		/* Kill this */
#define ktime_get_ts(x) do_posix_clock_monotonic_gettime(x)
/* Map the ktime_t to timespec conversion to ns_to_timespec function */
#define ktime_to_timespec(kt)		ns_to_timespec((kt).tv64)
#define ktime_get_ts64 ktime_get_ts
#include <linux/timekeeping.h>

#endif /* _LINUX_KTIME_H */
