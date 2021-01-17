/*
 * linux/mm/slab.h
 * Written by Mark Hemment, 1996.
 * (markhe@nextd.demon.co.uk)
 */

#include <linux/gfp.h>
#include <linux/overflow.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/workqueue.h>

#if	!defined(_LINUX_SLAB_H)
#define	_LINUX_SLAB_H

#if	defined(__KERNEL__)

typedef struct kmem_cache_s kmem_cache_t;

/* flags for kmem_cache_alloc() */
#define	SLAB_BUFFER		GFP_BUFFER
#define	SLAB_ATOMIC		GFP_ATOMIC
#define	SLAB_USER		GFP_USER
#define	SLAB_KERNEL		GFP_KERNEL
#define	SLAB_NFS		GFP_NFS
#define	SLAB_DMA		GFP_DMA

#define SLAB_LEVEL_MASK		0x0000007fUL
#define	SLAB_NO_GROW		0x00001000UL	/* don't grow a cache */

/* flags to pass to kmem_cache_create().
 * The first 3 are only valid when the allocator as been build
 * SLAB_DEBUG_SUPPORT.
 */
#define	SLAB_DEBUG_FREE		0x00000100UL	/* Peform (expensive) checks on free */
#define	SLAB_DEBUG_INITIAL	0x00000200UL	/* Call constructor (as verifier) */
#define	SLAB_RED_ZONE		0x00000400UL	/* Red zone objs in a cache */
#define	SLAB_POISON		0x00000800UL	/* Poison objects */
#define	SLAB_NO_REAP		0x00001000UL	/* never reap from the cache */
#define	SLAB_HWCACHE_ALIGN	0x00002000UL	/* align objs on a h/w cache lines */
#if	0
#define	SLAB_HIGH_PACK		0x00004000UL	/* XXX */
#endif

/* flags passed to a constructor func */
#define	SLAB_CTOR_CONSTRUCTOR	0x001UL		/* if not set, then deconstructor */
#define SLAB_CTOR_ATOMIC	0x002UL		/* tell constructor it can't sleep */
#define	SLAB_CTOR_VERIFY	0x004UL		/* tell constructor it's a verify call */

#endif	/* __KERNEL__ */

/*
 * ZERO_SIZE_PTR will be returned for zero sized kmalloc requests.
 *
 * Dereferencing ZERO_SIZE_PTR will lead to a distinct access fault.
 *
 * ZERO_SIZE_PTR can be passed to kfree though in the same way that NULL can.
 * Both make kfree a no-op.
 */
#define ZERO_SIZE_PTR ((void *)16)

//NOTE: enabling this in the non-KEE driver causes problems (file name strings
//      put in seperate private segments)
#ifdef DEBUGHEAP
extern void near *__kmalloc(int, int, const char *filename, int lineno);
extern void  __kfree(const void near *, const char *filename, int lineno);

static inline void *kmalloc(size_t size, gfp_t flags)
{
	return __kmalloc(size, flags, __FILE__, __LINE__);
}


#define kfree(a)                __kfree(a, __FILE__, __LINE__)
#define kfree_s(a,b)            __kfree(a, __FILE__, __LINE__)
#define kfree_nocheck(a)	__kfree(a, __FILE__, __LINE__)

#else
extern void near *__kmalloc(int, int);
extern void  __kfree(const void near *);

#define kmalloc(a,b)            __kmalloc(a,b)
#define kfree(a)                __kfree(a)

#define kfree_s(a,b)            kfree(a)
#define kfree_nocheck(a)	kfree(a)
#endif

void *kzalloc(size_t n, gfp_t gfp_flags);
void *kcalloc(size_t n, size_t size, unsigned int __nocast gfp_flags);
void *krealloc(const void *, size_t, gfp_t);

#define SIZE_MAX	(~(size_t)0)

/**
 * kmalloc_array - allocate memory for an array.
 * @n: number of elements.
 * @size: element size.
 * @flags: the type of memory to allocate (see kmalloc).
 */
static inline void *kmalloc_array(size_t n, size_t size, gfp_t flags)
{
	if (size != 0 && n > SIZE_MAX / size)
		return NULL;
	return __kmalloc(n * size, flags);
}

#define kmalloc_node_track_caller(size, flags, node) \
	kmalloc_track_caller(size, flags)
#define 	kmalloc_track_caller(size, flags)   __kmalloc(size, flags)
#define kvfree(arg)                     kfree(arg)

struct kmem_cache {
	unsigned int object_size;/* The original size of the object */
	unsigned int size;	/* The aligned/padded/added on size  */
	unsigned int align;	/* Alignment as calculated */
	unsigned long flags;	/* Active flags on the slab */
	const char *name;	/* Slab name for sysfs */
	int refcount;		/* Use counter */
	void (*ctor)(void *);	/* Called on object slot creation */
	struct list_head list;	/* List of all slab caches on the system */
};

#define kvzalloc kzalloc
size_t ksize(const void *);
#endif	/* _LINUX_SLAB_H */
