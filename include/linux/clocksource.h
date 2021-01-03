#ifndef _LINUX_CLOCKSOURCE_H
#define _LINUX_CLOCKSOURCE_H

#include <linux/types.h>

/* clocksource cycle base type */
typedef u64 cycle_t;
struct clocksource;


/**
 * struct cyclecounter - hardware abstraction for a free running counter
 *	Provides completely state-free accessors to the underlying hardware.
 *	Depending on which hardware it reads, the cycle counter may wrap
 *	around quickly. Locking rules (if necessary) have to be defined
 *	by the implementor and user of specific instances of this API.
 *
 * @read:		returns the current cycle value
 * @mask:		bitmask for two's complement
 *			subtraction of non 64 bit counters,
 *			see CLOCKSOURCE_MASK() helper macro
 * @mult:		cycle to nanosecond multiplier
 * @shift:		cycle to nanosecond divisor (power of two)
 */
struct cyclecounter {
	cycle_t (*read)(const struct cyclecounter *cc);
	cycle_t mask;
	u32 mult;
	u32 shift;
};

/**
 * struct timecounter - layer above a %struct cyclecounter which counts nanoseconds
 *	Contains the state needed by timecounter_read() to detect
 *	cycle counter wrap around. Initialize with
 *	timecounter_init(). Also used to convert cycle counts into the
 *	corresponding nanosecond counts with timecounter_cyc2time(). Users
 *	of this code are responsible for initializing the underlying
 *	cycle counter hardware, locking issues and reading the time
 *	more often than the cycle counter wraps around. The nanosecond
 *	counter will only wrap around after ~585 years.
 *
 * @cc:			the cycle counter used by this instance
 * @cycle_last:		most recent cycle counter value seen by
 *			timecounter_read()
 * @nsec:		continuously increasing count
 */
struct timecounter {
	const struct cyclecounter *cc;
	cycle_t cycle_last;
	u64 nsec;
};

/**
 * cyclecounter_cyc2ns - converts cycle counter cycles to nanoseconds
 * @cc:		Pointer to cycle counter.
 * @cycles:	Cycles
 *
 * XXX - This could use some mult_lxl_ll() asm optimization. Same code
 * as in cyc2ns, but with unsigned result.
 */
static inline u64 cyclecounter_cyc2ns(const struct cyclecounter *cc,
				      cycle_t cycles)
{
	u64 ret = (u64)cycles;
	ret = (ret * cc->mult) >> cc->shift;
	return ret;
}

/**
 * timecounter_init - initialize a time counter
 * @tc:			Pointer to time counter which is to be initialized/reset
 * @cc:			A cycle counter, ready to be used.
 * @start_tstamp:	Arbitrary initial time stamp.
 *
 * After this call the current cycle register (roughly) corresponds to
 * the initial time stamp. Every call to timecounter_read() increments
 * the time stamp counter by the number of elapsed nanoseconds.
 */
extern void timecounter_init(struct timecounter *tc,
			     const struct cyclecounter *cc,
			     u64 start_tstamp);

/**
 * timecounter_read - return nanoseconds elapsed since timecounter_init()
 *                    plus the initial time stamp
 * @tc:          Pointer to time counter.
 *
 * In other words, keeps track of time since the same epoch as
 * the function which generated the initial time stamp.
 */
extern u64 timecounter_read(struct timecounter *tc);

/* simplify initialization of mask field */
#define CLOCKSOURCE_MASK(bits) (cycle_t)((bits) < 64 ? ((1ULL<<(bits))-1) : -1)

#endif /* _LINUX_CLOCKSOURCE_H */
