#ifndef _LINUX_BYTEORDER_SWAB_H
#define _LINUX_BYTEORDER_SWAB_H

/*
 * linux/byteorder/swab.h
 * Byte-swapping, independently from CPU endianness
 *	swabXX[ps]?(foo)
 *
 * Francois-Rene Rideau <fare@tunes.org> 19971205
 *    separated swab functions from cpu_to_XX,
 *    to clean up support for bizarre-endian architectures.
 *
 * See asm-i386/byteorder.h and suches for examples of how to provide
 * architecture-dependent optimized versions
 *
 */

/* casts are necessary for constants, because we never know how for sure
 * how U/UL/ULL map to u16, u32, u64. At least not in a portable way.
 */
#define ___swab16(x) \
	((u16)( \
		(((u16)(x) & (u16)0x00ffU) << 8) | \
		(((u16)(x) & (u16)0xff00U) >> 8) ))
#define ___swab32(x) \
	((u32)( \
		(((u32)(x) & (u32)0x000000ffUL) << 24) | \
		(((u32)(x) & (u32)0x0000ff00UL) <<  8) | \
		(((u32)(x) & (u32)0x00ff0000UL) >>  8) | \
		(((u32)(x) & (u32)0xff000000UL) >> 24) ))
#define ___swab64(x) \
	((u64)( \
		(u64)(((u64)(x) & (u64)0x00000000000000ff) << 56) | \
		(u64)(((u64)(x) & (u64)0x000000000000ff00) << 40) | \
		(u64)(((u64)(x) & (u64)0x0000000000ff0000) << 24) | \
		(u64)(((u64)(x) & (u64)0x00000000ff000000) <<  8) | \
	        (u64)(((u64)(x) & (u64)0x000000ff00000000) >>  8) | \
		(u64)(((u64)(x) & (u64)0x0000ff0000000000) >> 24) | \
		(u64)(((u64)(x) & (u64)0x00ff000000000000) >> 40) | \
		(u64)(((u64)(x) & (u64)0xff00000000000000) >> 56) ))

/*
 * provide defaults when no architecture-specific optimization is detected
 */
#ifndef __arch__swab16
#  define __arch__swab16(x) ___swab16(x)
#endif
#ifndef __arch__swab32
#  define __arch__swab32(x) ___swab32(x)
#endif
#ifndef __arch__swab64
#  define __arch__swab64(x) ___swab64(x)
#endif

#ifndef __arch__swab16p
#  define __arch__swab16p(x) __swab16(*(x))
#endif
#ifndef __arch__swab32p
#  define __arch__swab32p(x) __swab32(*(x))
#endif
#ifndef __arch__swab64p
#  define __arch__swab64p(x) __swab64(*(x))
#endif

#ifndef __arch__swab16s
#  define __arch__swab16s(x) do { *(x) = __swab16p((x)); } while (0)
#endif
#ifndef __arch__swab32s
#  define __arch__swab32s(x) do { *(x) = __swab32p((x)); } while (0)
#endif
#ifndef __arch__swab64s
#  define __arch__swab64s(x) do { *(x) = __swab64p((x)); } while (0)
#endif


/*
 * Allow constant folding
 */
#if defined(__GNUC__) && (__GNUC__ >= 2) && defined(__OPTIMIZE__)
#  define __swab16(x) \
(__builtin_constant_p((u16)(x)) ? \
 ___swab16((x)) : \
 __fswab16((x)))
#  define __swab32(x) \
(__builtin_constant_p((u32)(x)) ? \
 ___swab32((x)) : \
 __fswab32((x)))
#  define __swab64(x) \
(__builtin_constant_p((u64)(x)) ? \
 ___swab64((x)) : \
 __fswab64((x)))
#else
#  define __swab16(x) ___swab16(x)
#  define __swab32(x) ___swab32(x)
#  define __swab64(x) ___swab64(x)
#endif /* OPTIMIZE */


#ifdef __BYTEORDER_HAS_U64__
extern __inline__ __const__ u64 __fswab64(u64 x)
{
#  ifdef __SWAB_64_THRU_32__
	u32 h = x >> 32;
        u32 l = x & ((1ULL<<32)-1);
        return (((u64)__swab32(l)) << 32) | ((u64)(__swab32(h)));
#  else
	return __arch__swab64(x);
#  endif
}
#endif /* __BYTEORDER_HAS_U64__ */

#if defined(__KERNEL__)
#define swab16 __swab16
#define swab32 __swab32
#define swab64 __swab64
#define swab16p __swab16p
#define swab32p __swab32p
#define swab64p __swab64p
#define swab16s __swab16s
#define swab32s __swab32s
#define swab64s __swab64s
#endif

#endif /* _LINUX_BYTEORDER_SWAB_H */
