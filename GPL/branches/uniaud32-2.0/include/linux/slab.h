/*
 * linux/mm/slab.h
 * Written by Mark Hemment, 1996.
 * (markhe@nextd.demon.co.uk)
 */

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

//NOTE: enabling this in the non-KEE driver causes problems (file name strings
//      put in seperate private segments)
#ifdef DEBUGHEAP
extern void near *__kmalloc(int, int, const char *filename, int lineno);
extern void  __kfree(const void near *, const char *filename, int lineno);

#define kmalloc(a,b)            __kmalloc(a,b, __FILE__, __LINE__)
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

#endif	/* _LINUX_SLAB_H */
