#ifndef _LINUX_LOG2_H
#define _LINUX_LOG2_H

#include <linux/bitops.h>
#include <linux/types.h>

/*
 *  Determine whether some value is a power of two, where zero is
 * *not* considered a power of two.
 */

static inline 
bool is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

/**
 * __roundup_pow_of_two() - round up to nearest power of two
 * @n: value to round up
 */
static inline /*__attribute__((const))*/
unsigned long __roundup_pow_of_two(unsigned long n)
{
	return 1UL << fls_long(n - 1);
}

/**
 * __rounddown_pow_of_two() - round down to nearest power of two
 * @n: value to round down
 */
static inline /*__attribute__((const))*/
unsigned long __rounddown_pow_of_two(unsigned long n)
{
	return 1UL << (fls_long(n) - 1);
}

/***********************************************/
/* Locate the position of the highest bit set. */
/* A binary search is used.  The result is an  */
/* approximation of log2(n) [the integer part] */
/***********************************************/
static inline int             ilog2(unsigned long n)
{
    int             i = (-1);

    /* Is there a bit on in the high word? */
    /* Else, all the high bits are already zero. */
    if (n & 0xffff0000) {
        i += 16;                /* Update our search position */
        n >>= 16;               /* Shift out lower (irrelevant) bits */
    }
    /* Is there a bit on in the high byte of the current word? */
    /* Else, all the high bits are already zero. */
    if (n & 0xff00) {
        i += 8;                 /* Update our search position */
        n >>= 8;                /* Shift out lower (irrelevant) bits */
    }
    /* Is there a bit on in the current nybble? */
    /* Else, all the high bits are already zero. */
    if (n & 0xf0) {
        i += 4;                 /* Update our search position */
        n >>= 4;                /* Shift out lower (irrelevant) bits */
    }
    /* Is there a bit on in the high 2 bits of the current nybble? */
    /* 0xc is 1100 in binary... */
    /* Else, all the high bits are already zero. */
    if (n & 0xc) {
        i += 2;                 /* Update our search position */
        n >>= 2;                /* Shift out lower (irrelevant) bits */
    }
    /* Is the 2nd bit on? [ 0x2 is 0010 in binary...] */
    /* Else, all the 2nd bit is already zero. */
    if (n & 0x2) {
        i++;                    /* Update our search position */
        n >>= 1;                /* Shift out lower (irrelevant) bit */
    }
    /* Is the lowest bit set? */
    if (n)
        i++;                    /* Update our search position */
    return i;
}

/**
 * roundup_pow_of_two - round the given value up to nearest power of two
 * @n: parameter
 *
 * round the given value up to the nearest power of two
 * - the result is undefined when n == 0
 * - this can be used to initialise global variables from constant data
 */
#define roundup_pow_of_two(n)			\
(						\
	__roundup_pow_of_two(n)			\
 )

/**
 * rounddown_pow_of_two - round the given value down to nearest power of two
 * @n: parameter
 *
 * round the given value down to the nearest power of two
 * - the result is undefined when n == 0
 * - this can be used to initialise global variables from constant data
 */
#define rounddown_pow_of_two(n)			\
(						\
	__rounddown_pow_of_two(n)		\
 )

#endif /* _LINUX_LOG2_H */
