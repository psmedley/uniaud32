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

int atomic_dec_and_test(volatile atomic_t *v);
extern int atomic_add_negative(int i, volatile atomic_t *v);

/* These are x86-specific, used by some header files */
#define atomic_clear_mask(mask, addr) 

#define atomic_set_mask(mask, addr) 

#endif
