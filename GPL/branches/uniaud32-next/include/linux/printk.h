#ifndef _LINUX_PRINTK_H
#define _LINUX_PRINTK_H
#if DEBUG
#define pr_debug printk
#define pr_err printk
#define pr_info printk
#define pr_warning printk
#define pr_warn pr_warning
#else
#define pr_debug
#define pr_err 
#define pr_info printk
#define pr_warning printk
#define pr_warn pr_warning
#endif

#endif /* _LINUX_PRINTK_H */
