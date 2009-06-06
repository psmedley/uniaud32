#ifndef _I386_BITOPS_H
#define _I386_BITOPS_H

/*
 * Copyright 1992, Linus Torvalds.
 */

/*
 * These have to be done with inline assembly: that way the bit-setting
 * is guaranteed to be atomic. All bit operations return 0 if the bit
 * was cleared before the operation and != 0 if it was not.
 *
 * bit 0 is the LSB of addr; bit 32 is the LSB of (addr+1).
 */

/*
 * Function prototypes to keep gcc -Wall happy
 */
#include <asm\bitops.h>
#define smp_mb__after_clear_bit()	
#endif /* _I386_BITOPS_H */
