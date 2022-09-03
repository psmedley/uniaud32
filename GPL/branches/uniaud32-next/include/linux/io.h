#ifndef _LINUX_IO_H
#define _LINUX_IO_H

#include <asm/io.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/err.h>

void __iomem *devm_ioremap(struct device *dev, resource_size_t offset,
			   resource_size_t size);

#endif /* _LINUX_IO_H */
