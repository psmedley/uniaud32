#ifndef __ARCH_I386_ATOMIC__
#define __ARCH_I386_ATOMIC__

#define LOCK

typedef struct { int counter; } atomic_t;

#define ATOMIC_INIT(i)	{ (i) }

#define atomic_read(v)		((v)->counter)
#define atomic_set(v,i)		(((v)->counter) = (i))

void atomic_add(int i, volatile atomic_t *v);
void atomic_sub(int i, volatile atomic_t *v);

void atomic_inc(volatile atomic_t *v);
#pragma aux atomic_inc =       \
  "lock inc dword ptr [eax]"                  \
  parm [eax];

void atomic_dec(volatile atomic_t *v);
#pragma aux atomic_dec =       \
  "lock dec dword ptr [eax]"                  \
  parm [eax];

/**
 * atomic_dec_and_test - decrement and test
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.  Note that the guaranteed
 * useful range of an atomic_t is only 24 bits.
 */
static inline int atomic_dec_and_test(volatile atomic_t *v)
{
    atomic_dec(v);
    if (v->counter == 0)
        return 1;
    return 0;
}

extern int atomic_add_negative(int i, volatile atomic_t *v);

/* These are x86-specific, used by some header files */
#define atomic_clear_mask(mask, addr) 

/**
 * atomic_add_return - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v and returns the result
 */
static inline int atomic_add_return(int i, atomic_t *v)
{
	//unsigned long flags;
	int temp;

	//raw_local_irq_save(flags); /* Don't trace it in an irqsoff handler */
	temp = v->counter;
	temp += i;
	v->counter = temp;
	//raw_local_irq_restore(flags);

	return temp;
}

/**
 * atomic_sub_return - subtract integer from atomic variable
 * @i: integer value to subtract
 * @v: pointer of type atomic_t
 *
 * Atomically subtracts @i from @v and returns the result
 */
static inline int atomic_sub_return(int i, atomic_t *v)
{
	//unsigned long flags;
	int temp;

	//raw_local_irq_save(flags); /* Don't trace it in an irqsoff handler */
	temp = v->counter;
	temp -= i;
	v->counter = temp;
	//raw_local_irq_restore(flags);

	return temp;
}

#define atomic_set_mask(mask, addr) 
#define atomic_sub_and_test(i, v)	(atomic_sub_return((i), (v)) == 0)
#define atomic_inc_return(v)		atomic_add_return(1, (v))

typedef struct {
	long long counter;
} atomic64_t;

typedef atomic64_t atomic_long_t;
#endif
