#ifndef _LINUX_EXPORT_H
#define _LINUX_EXPORT_H

extern int this_module[64];
#define THIS_MODULE (void *)&this_module[0]
#define EXPORT_SYMBOL(a)
#define EXPORT_SYMBOL_GPL(a)

#endif /* _LINUX_EXPORT_H */
