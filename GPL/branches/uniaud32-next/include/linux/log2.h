#ifndef _LINUX_LOG2_H
#define _LINUX_LOG2_H

/*
 *  Determine whether some value is a power of two, where zero is
 * *not* considered a power of two.
 */

static inline 
bool is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

/*
 * round up to nearest power of two
 */
static inline 
unsigned long __roundup_pow_of_two(unsigned long n)
{
	return 1UL << fls_long(n - 1);
}

/*
 * round down to nearest power of two
 */
static inline 
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

#endif /* _LINUX_LOG2_H */
