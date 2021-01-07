#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

/*
 * 'kernel.h' contains some often-used function prototypes etc
 */

#ifdef __KERNEL__

#include <stdlib.h>
#include <stdarg.h>
//#include <linux/linkage.h>
#include <linux/gfp.h>
#include <linux/types.h>

/* Optimization barrier */
/* The "volatile" is due to gcc bugs */
//#define barrier() __asm__ __volatile__("": : :"memory")
#define barrier()

#define STACK_MAGIC	0xdeadbeef

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define	KERN_EMERG	"<0>"	/* system is unusable			*/
#define	KERN_ALERT	"<1>"	/* action must be taken immediately	*/
#define	KERN_CRIT	"<2>"	/* critical conditions			*/
#define	KERN_ERR	"<3>"	/* error conditions			*/
#define	KERN_WARNING	"<4>"	/* warning conditions			*/
#define	KERN_NOTICE	"<5>"	/* normal but significant condition	*/
#define	KERN_INFO	"<6>"	/* informational			*/
#define	KERN_DEBUG	"<7>"	/* debug-level messages			*/

# define NORET_TYPE    /**/
# define ATTRIB_NORET  __attribute__((noreturn))
# define NORET_AND     noreturn,

#ifdef __i386__
#define FASTCALL(x)	x __attribute__((regparm(3)))
#else
#define FASTCALL(x)	x
#endif

extern void math_error(void);
extern struct notifier_block *panic_notifier_list;
NORET_TYPE void panic(const char * fmt, ...);

NORET_TYPE void do_exit(long error_code);

#define simple_strtoul strtoul
extern long simple_strtol(const char *,char **,unsigned int);
extern int sprintf(char * buf, const char * fmt, ...);
extern int vsprintf(char *buf, const char *, va_list);
extern int get_option(char **str, int *pint);
extern char *get_options(char *str, int nints, int *ints);

extern int session_of_pgrp(int pgrp);

int cdecl printk(const char * fmt, ...);


/*
 *      Display an IP address in readable format.
 */

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#endif /* __KERNEL__ */

#define SI_LOAD_SHIFT	16
struct sysinfo {
	long uptime;			/* Seconds since boot */
	unsigned long loads[3];		/* 1, 5, and 15 minute load averages */
	unsigned long totalram;		/* Total usable main memory size */
	unsigned long freeram;		/* Available memory size */
	unsigned long sharedram;	/* Amount of shared memory */
	unsigned long bufferram;	/* Memory used by buffers */
	unsigned long totalswap;	/* Total swap space size */
	unsigned long freeswap;		/* swap space still available */
	unsigned short procs;		/* Number of current processes */
	unsigned long totalhigh;	/* Total high memory size */
	unsigned long freehigh;		/* Available high memory size */
	unsigned int mem_unit;		/* Memory unit size in bytes */
	char _f[20-2*sizeof(long)-sizeof(int)];	/* Padding: libc5 uses this.. */
};

static void complete_and_exit(struct completion *, long);
#define printk_ratelimit()      1
#define ALIGN(x,a) (((x)+(a)-1)&~((a)-1))
#define dump_stack()
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
int strict_strtoul(const char *, unsigned int, unsigned long *);

#define BUG_ON(condition)
#define WARN_ON(condition) (void)0
#define WARN_ON_ONCE(condition) (void)0
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
#define SIZE_MAX	(~(size_t)0)
_WCRTLINK extern int     vsnprintf( char *__s, size_t __bufsize,
                                    const char *__format, __va_list __arg );
char *kasprintf(gfp_t gfp, const char *fmt, ...);

char *kvasprintf(gfp_t gfp, const char *fmt, va_list args);
extern int hex_to_bin(char ch);
#define __ALIGN_MASK(x, mask)	__ALIGN_KERNEL_MASK((x), (mask))
#define __ALIGN_KERNEL_MASK(x, mask)	(((x) + (mask)) & ~(mask))

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) \
( (type *)( (char *)ptr - offsetof(type,member) ) )
#endif
