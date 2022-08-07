#ifndef _LINUX_IO_H
#define _LINUX_IO_H

#include <asm/io.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/err.h>

#define devm_ioremap(A, B, C) ioremap(B, C)
#endif /* _LINUX_IO_H */
