#ifndef _LINUX_TIME_H
#define _LINUX_TIME_H

#include <asm/param.h>
#include <linux/types.h>
#include <linux/ktime.h>
#include <linux/time64.h>
#include <linux/math64.h>

#define NSEC_PER_SEC	1000000000L


#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC
struct timespec {
	time_t	tv_sec;		/* seconds */
	long	tv_nsec;	/* nanoseconds */
};
#endif /* _STRUCT_TIMESPEC */

static inline int timespec_equal(const struct timespec *a,
                                 const struct timespec *b)
{
	return (a->tv_sec == b->tv_sec) && (a->tv_nsec == b->tv_nsec);
}

/*
 * Change timeval to jiffies, trying to avoid the
 * most obvious overflows..
 *
 * And some not so obvious.
 *
 * Note that we don't want to return MAX_LONG, because
 * for various timeout reasons we often end up having
 * to wait "jiffies+1" in order to guarantee that we wait
 * at _least_ "jiffies" - so "jiffies+1" had better still
 * be positive.
 */
#define MAX_JIFFY_OFFSET ((~0UL >> 1)-1)

static __inline__ unsigned long
timespec_to_jiffies(struct timespec *value)
{
	unsigned long sec = value->tv_sec;
	long nsec = value->tv_nsec;

	if (sec >= (MAX_JIFFY_OFFSET / HZ))
		return MAX_JIFFY_OFFSET;
	nsec += 1000000000L / HZ - 1;
	nsec /= 1000000000L / HZ;
	return HZ * sec + nsec;
}

static __inline__ void
jiffies_to_timespec(unsigned long jiffies, struct timespec *value)
{
	value->tv_nsec = (jiffies % HZ) * (1000000000L / HZ);
	value->tv_sec = jiffies / HZ;
}

struct timeval {
	time_t		tv_sec;		/* seconds */
	suseconds_t	tv_usec;	/* microseconds */
};

struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};

#define NFDBITS			__NFDBITS

#ifdef __KERNEL__
extern void do_gettimeofday(struct timeval *tv);
extern void do_settimeofday(struct timeval *tv);
extern void get_fast_time(struct timeval *tv);
extern void (*do_get_fast_time)(struct timeval *);
#endif

#define FD_SETSIZE		__FD_SETSIZE
#define FD_SET(fd,fdsetp)	__FD_SET(fd,fdsetp)
#define FD_CLR(fd,fdsetp)	__FD_CLR(fd,fdsetp)
#define FD_ISSET(fd,fdsetp)	__FD_ISSET(fd,fdsetp)
#define FD_ZERO(fdsetp)		__FD_ZERO(fdsetp)

/*
 * Names of the interval timers, and structure
 * defining a timer setting.
 */
#define	ITIMER_REAL	0
#define	ITIMER_VIRTUAL	1
#define	ITIMER_PROF	2

struct  itimerspec {
        struct  timespec it_interval;    /* timer period */
        struct  timespec it_value;       /* timer expiration */
};

struct	itimerval {
	struct	timeval it_interval;	/* timer interval */
	struct	timeval it_value;	/* current value */
};

/* msecs_to_jiffies */
#ifndef CONFIG_HAVE_MSECS_TO_JIFFIES
static __inline__ unsigned int jiffies_to_msecs(const unsigned long j)
{
#if (HZ <= 1000 && !(1000 % HZ))
		return (1000 / HZ) * j;
#elif (HZ > 1000 && !(HZ % 1000))
		return (j + (HZ / 1000) - 1)/(HZ / 1000);
#else
		return (j * 1000) / HZ;
#endif
}
static __inline__ unsigned long msecs_to_jiffies(const unsigned int m)
{
	if (m > jiffies_to_msecs(MAX_JIFFY_OFFSET))
		return MAX_JIFFY_OFFSET;
#if (HZ <= 1000 && !(1000 % HZ))
		return (m + (1000 / HZ) - 1) / (1000 / HZ);
#elif (HZ > 1000 && !(HZ % 1000))
		return m * (HZ / 1000);
#else
		return (m * HZ + 999) / 1000;
#endif
}
#endif

/* wrapper for getnstimeofday()
 * it's needed for recent 2.6 kernels, too, due to lack of EXPORT_SYMBOL
 */
#define getnstimeofday(x) do { \
	struct timeval __x; \
	do_gettimeofday(&__x); \
	(x)->tv_sec = __x.tv_sec;	\
	(x)->tv_nsec = __x.tv_usec * 1000; \
} while (0)

#define do_posix_clock_monotonic_gettime getnstimeofday

void msleep(unsigned int msecs);

/**
 * ns_to_timespec - Convert nanoseconds to timespec
 * @nsec:	the nanoseconds value to be converted
 *
 * Returns the timespec representation of the nsec parameter.
 */
extern struct timespec ns_to_timespec(const s64 nsec);
#define getrawmonotonic(ts) do_posix_clock_monotonic_gettime(ts)
#endif
