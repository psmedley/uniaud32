#ifndef _LINUX_TIME64_H
#define _LINUX_TIME64_H

typedef __s64 time64_t;
typedef __u64 timeu64_t;

struct timespec64 {
	time64_t	tv_sec;			/* seconds */
	long		tv_nsec;		/* nanoseconds */
};

struct itimerspec64 {
	struct timespec64 it_interval;
	struct timespec64 it_value;
};

/* Parameters used to convert the timespec values: */
#define MSEC_PER_SEC	1000L
#define USEC_PER_MSEC	1000L
#define NSEC_PER_USEC	1000L
#define NSEC_PER_MSEC	1000000L
#define USEC_PER_SEC	1000000L
#define NSEC_PER_SEC	1000000000L
#define FSEC_PER_SEC	1000000000000000LL

/* Located here for timespec[64]_valid_strict */
#define TIME64_MAX			((s64)~((u64)1 << 63))
#define KTIME_MAX			((s64)~((u64)1 << 63))
#define KTIME_SEC_MAX			(KTIME_MAX / NSEC_PER_SEC)

extern void set_normalized_timespec64(struct timespec64 *ts, time64_t sec, s64 nsec);

static inline struct timespec64 timespec64_add(struct timespec64 lhs,
						struct timespec64 rhs)
{
	struct timespec64 ts_delta;
	set_normalized_timespec64(&ts_delta, lhs.tv_sec + rhs.tv_sec,
				lhs.tv_nsec + rhs.tv_nsec);
	return ts_delta;
}

/*
 * sub = lhs - rhs, in normalized form
 */
static inline struct timespec64 timespec64_sub(struct timespec64 lhs,
						struct timespec64 rhs)
{
	struct timespec64 ts_delta;
	set_normalized_timespec64(&ts_delta, lhs.tv_sec - rhs.tv_sec,
				lhs.tv_nsec - rhs.tv_nsec);
	return ts_delta;
}

/*
 * Returns true if the timespec64 is norm, false if denorm:
 */
static inline bool timespec64_valid(const struct timespec64 *ts)
{
	/* Dates before 1970 are bogus */
	if (ts->tv_sec < 0)
		return false;
	/* Can't have more nanoseconds then a second */
	if ((unsigned long)ts->tv_nsec >= NSEC_PER_SEC)
		return false;
	return true;
}

static inline bool timespec64_valid_strict(const struct timespec64 *ts)
{
	if (!timespec64_valid(ts))
		return false;
	/* Disallow values that could overflow ktime_t */
	if ((unsigned long long)ts->tv_sec >= KTIME_SEC_MAX)
		return false;
	return true;
}

/**
 * timespec64_to_ns - Convert timespec64 to nanoseconds
 * @ts:		pointer to the timespec64 variable to be converted
 *
 * Returns the scalar nanosecond representation of the timespec64
 * parameter.
 */
static inline s64 timespec64_to_ns(const struct timespec64 *ts)
{
	return ((s64) ts->tv_sec * NSEC_PER_SEC) + ts->tv_nsec;
}

/**
 * ns_to_timespec64 - Convert nanoseconds to timespec64
 * @nsec:	the nanoseconds value to be converted
 *
 * Returns the timespec64 representation of the nsec parameter.
 */
extern struct timespec64 ns_to_timespec64(const s64 nsec);

#endif
