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
#ifdef __WATCOMC__
int set_bit(int nr, volatile void * addr);
#pragma aux set_bit =     \
  "bts dword ptr [esi], eax"     \
  "setc al"          \
  "movzx eax, al"    \
  parm [eax] [esi]   \
  value [eax]

int clear_bit(int nr, volatile void * addr);
#pragma aux clear_bit =     \
  "btr dword ptr [esi], eax"     \
  "setc al"          \
  "movzx eax, al"    \
  parm [eax] [esi]   \
  value [eax]

int change_bit(int nr, volatile void * addr);
#pragma aux change_bit =     \
  "btc dword ptr [esi], eax"     \
  "setc al"          \
  "movzx eax, al"    \
  parm [eax] [esi]   \
  value [eax]

#define test_and_set_bit(nr, addr)    set_bit(nr, addr)
#define test_and_clear_bit(nr, addr)  clear_bit(nr, addr)
#define test_and_change_bit(nr, addr) change_bit(nr, addr)

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */

int ffs(int x);
#pragma aux ffs =    \
  "bsf eax, ebx"     \
  "jnz @f"           \
  "mov eax, -1"      \
  "@f:"              \
  "inc eax"          \
  parm [ebx]         \
  value [eax]

/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 */
unsigned long ffz(unsigned long word);
#pragma aux ffz =    \
  "bsf eax, ebx"     \
  parm [ebx]         \
  value [eax]

//{
//	int r;
//
//	__asm__("bsfl %1,%0\n\t"
//		"jnz 1f\n\t"
//		"movl $-1,%0\n"
//		"1:" : "=r" (r) : "g" (x));
//	return r+1;
//}

#else
extern void set_bit(int nr, volatile void * addr);
extern void clear_bit(int nr, volatile void * addr);
extern void change_bit(int nr, volatile void * addr);
extern int test_and_set_bit(int nr, volatile void * addr);
extern int test_and_clear_bit(int nr, volatile void * addr);
extern int test_and_change_bit(int nr, volatile void * addr);
#endif

extern int __constant_test_bit(int nr, const volatile void * addr);
extern int __test_bit(int nr, volatile void * addr);
extern int find_first_zero_bit(void * addr, unsigned size);
extern int find_next_zero_bit (void * addr, int size, int offset);

/*
 * This routine doesn't need to be atomic.
 */
#define test_bit(nr, addr) (((1UL << (nr & 31)) & (((const unsigned int *) addr)[nr >> 5])) != 0)

#endif /* _I386_BITOPS_H */
