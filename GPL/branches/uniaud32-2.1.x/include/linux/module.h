/*
 * Dynamic loading of modules into the kernel.
 *
 * Rewritten by Richard Henderson <rth@tamu.edu> Dec 1996
 */

#ifndef _LINUX_MODULE_H
#define _LINUX_MODULE_H

/* Poke the use count of a module.  */

#define __MOD_DEC_USE_COUNT(mod)		\
	do {} while(0);

#define MOD_INC_USE_COUNT
#define MOD_DEC_USE_COUNT
#define MOD_IN_USE

#define EXPORT_NO_SYMBOLS

#define __MODULE_STRING_1(x)	#x
#define __MODULE_STRING(x)	__MODULE_STRING_1(x)

/* For documentation purposes only.  */

#define MODULE_AUTHOR(name)						   

#define MODULE_DESCRIPTION(desc)					   

/* Could potentially be used by kmod...  */

#define MODULE_SUPPORTED_DEVICE(dev)					   

/* Used to verify parameters given to the module.  The TYPE arg should
   be a string in the following format:
   	[min[-max]]{b,h,i,l,s}
   The MIN and MAX specifiers delimit the length of the array.  If MAX
   is omitted, it defaults to MIN; if both are omitted, the default is 1.
   The final character is a type specifier:
	b	byte
	h	short
	i	int
	l	long
	s	string
*/

#ifdef TARGET_OS2
#define MODULE_PARM(var,type)
#define MODULE_PARM_DESC(var,desc)
#define MODULE_LICENSE(a)
#else
#define MODULE_PARM(var,type)			\
const char __module_parm_##var[]=		\
"parm_" __MODULE_STRING(var) "=" type

#define MODULE_PARM_DESC(var,desc)		\
const char __module_parm_desc_##var[]=		\
"parm_desc_" __MODULE_STRING(var) "=" desc
#endif

#define try_inc_mod_count(x) 	        ++(*(unsigned long *)x)
#define try_module_get(x) try_inc_mod_count(x)
static inline void module_put(struct module *module)
{
    if (module)
        do {} while(0);
}

#define MODULE_FIRMWARE(x)
#define MODULE_ALIAS(x)

extern int this_module[64];
#define THIS_MODULE (void *)&this_module[0]
#define MODULE_GENERIC_TABLE(gtype,name)
#define MODULE_DEVICE_TABLE(type,name)
#define EXPORT_SYMBOL(a)

#endif /* _LINUX_MODULE_H */
