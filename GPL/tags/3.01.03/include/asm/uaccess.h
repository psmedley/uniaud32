/* $Id: uaccess.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef __i386_UACCESS_H
#define __i386_UACCESS_H

/*
 * User space memory access functions
 */
//#include <linux/config.h>
//#include <linux/sched.h>
#include <asm/page.h>

#define VERIFY_READ 0
#define VERIFY_WRITE 1

/*
 * The fs value determines whether argument validity checking should be
 * performed or not.  If get_fs() == USER_DS, checking is performed, with
 * get_fs() == KERNEL_DS, checking is bypassed.
 *
 * For historical reasons, these macros are grossly misnamed.
 */

#define MAKE_MM_SEG(s)	(mm_segment_t)(s)


#define KERNEL_DS	MAKE_MM_SEG(0xFFFFFFFF)
#define USER_DS		MAKE_MM_SEG(PAGE_OFFSET)

#define get_ds()	(KERNEL_DS)
#define get_fs()	(KERNEL_DS)
#define set_fs(x)	

#define segment_eq(a,b)	((a).seg == (b).seg)

extern int __verify_write(const void *, unsigned long);

#define __addr_ok(addr) ((unsigned long)(addr) < (current->addr_limit.seg))

/*
 * Uhhuh, this needs 33-bit arithmetic. We have a carry..
 */

int is_access_ok(int type, void *addr, unsigned long size);

#define access_ok(type, addr, size) __access_ok((unsigned long)(addr),(size))

/*
 * The architecture should really override this if possible, at least
 * doing a check on the get_fs()
 */
static inline int __access_ok(unsigned long addr, unsigned long size)
{
	return 1;
}

#define verify_area(type, addr, size) (access_ok(type, (void *)addr,size) ? 0 : -EFAULT)


/*
 * The exception table consists of pairs of addresses: the first is the
 * address of an instruction that is allowed to fault, and the second is
 * the address at which the program should continue.  No registers are
 * modified, so it is entirely up to the continuation code to figure out
 * what to do.
 *
 * All the routines below use bits of fixup code that are out of line
 * with the main instruction path.  This means when everything is well,
 * we don't even have to jump over them.  Further, they do not intrude
 * on our cache or tlb entries.
 */

struct exception_table_entry
{
	unsigned long insn, fixup;
};

/* Returns 0 if exception not found and fixup otherwise.  */
extern unsigned long search_exception_table(unsigned long);


/*
 * These are the main single-value transfer routines.  They automatically
 * use the right size if we just have the right pointer type.
 *
 * This gets kind of ugly. We want to return _two_ values in "get_user()"
 * and yet we don't want to do any pointers, because that is too much
 * of a performance impact. Thus we have a few rather ugly macros here,
 * and hide all the uglyness from the user.
 *
 * The "__xxx" versions of the user access functions are versions that
 * do not verify the address space, that must have been done previously
 * with a separate "access_ok()" call (this is used when we do multiple
 * accesses to the same area of user memory).
 */

/* Careful: we have to cast the result to the type of the pointer for sign reasons */
int __get_user(int size, void *dest, void *src);

extern void __put_user_bad(void);

#define get_user(x,ptr) \
  __get_user(sizeof(*ptr), &x,(ptr))

int _put_user(int size, int x, void *ptr);

#define put_user(x,ptr) \
  _put_user(sizeof(*ptr), x, (ptr))

#define __put_user(x,ptr) \
  _put_user(sizeof(*ptr), x, (ptr))

#define __put_user_nocheck(x,ptr,size)			lksjdflksdjf

#define __put_user_size(x,ptr,size,retval)		asdlkfjsdklfj

struct __large_struct { unsigned long buf[100]; };
#define __m(x) (*(struct __large_struct *)(x))

/*
 * Tell gcc we read from memory instead of writing: this is because
 * we do not write to any memory gcc knows about, so there are no
 * aliasing issues.
 */
#define __put_user_asm(x, addr, err, itype, rtype, ltype)	lksjdf


#define __get_user_nocheck(x,ptr,size)				lsjdf

extern long __get_user_bad(void);

#define __get_user_size(x,ptr,size,retval)		ksjdf

#define __get_user_asm(x, addr, err, itype, rtype, ltype)	sldkjf

/*
 * The "xxx_ret" versions return constant specified in third argument, if
 * something bad happens. These macros can be optimized for the
 * case of just returning from the function xxx_ret is used.
 */

#define put_user_ret(x,ptr,ret) ({ if (put_user(x,ptr)) return ret; })

#define get_user_ret(x,ptr,ret) if (get_user(sizeof(x), (void *)&x, (void *)ptr)) return ret;


/*
 * Copy To/From Userspace
 */

/* Generic arbitrary sized copy.  */
void __copy_user(void *to, const void *from, unsigned long n);

void __copy_user_zeroing(void *to, const void *from, unsigned long n);

/* We let the __ versions of copy_from/to_user inline, because they're often
 * used in fast paths and have only a small space overhead.
 */
static __inline unsigned long
__generic_copy_from_user_nocheck(void *to, const void *from, unsigned long n)
{
	__copy_user_zeroing(to,from,n);
	return n;
}

static __inline unsigned long
__generic_copy_to_user_nocheck(void *to, const void *from, unsigned long n)
{
	__copy_user(to,from,n);
	return n;
}


/* Optimize just a little bit when we know the size of the move. */
void __constant_copy_user(void *to, const void *from, unsigned long n);

/* Optimize just a little bit when we know the size of the move. */
void __constant_copy_user_zeroing(void *to, const void *from, unsigned long n);

unsigned long __generic_copy_to_user(void *, const void *, unsigned long);
unsigned long __generic_copy_from_user(void *, const void *, unsigned long);

#if 0
static __inline unsigned long
__constant_copy_to_user(void *to, const void *from, unsigned long n)
{
	if (access_ok(VERIFY_WRITE, to, n))
		__constant_copy_user(to,from,n);
	return n;
}

static __inline unsigned long
__constant_copy_from_user(void *to, const void *from, unsigned long n)
{
	if (access_ok(VERIFY_READ, (void *)from, n))
		__constant_copy_user_zeroing(to,from,n);
	return n;
}
#endif

static __inline unsigned long
__constant_copy_to_user_nocheck(void *to, const void *from, unsigned long n)
{
	__constant_copy_user(to,from,n);
	return n;
}

static __inline unsigned long
__constant_copy_from_user_nocheck(void *to, const void *from, unsigned long n)
{
	__constant_copy_user_zeroing(to,from,n);
	return n;
}

unsigned long copy_to_user(void *to, const void *from, unsigned long n);

unsigned long copy_from_user(void *to, const void *from, unsigned long n);

#define copy_to_user_ret(to,from,n,retval) ({ if (copy_to_user(to,from,n)) return retval; })

#define copy_from_user_ret(to,from,n,retval) ({ if (copy_from_user(to,from,n)) return retval; })

#define __copy_to_user(to,from,n)			\
	(__builtin_constant_p(n) ?			\
	 __constant_copy_to_user_nocheck((to),(from),(n)) :	\
	 __generic_copy_to_user_nocheck((to),(from),(n)))

#define __copy_from_user(to,from,n)			\
	(__builtin_constant_p(n) ?			\
	 __constant_copy_from_user_nocheck((to),(from),(n)) :	\
	 __generic_copy_from_user_nocheck((to),(from),(n)))

long strncpy_from_user(char *dst, const char *src, long count);
long __strncpy_from_user(char *dst, const char *src, long count);
#define strlen_user(str) strnlen_user(str, ~0UL >> 1)
long strnlen_user(const char *str, long n);
unsigned long clear_user(void *mem, unsigned long len);
unsigned long __clear_user(void *mem, unsigned long len);

#endif /* __i386_UACCESS_H */
