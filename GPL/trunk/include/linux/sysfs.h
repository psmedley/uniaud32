#ifndef _LINUX_SYSFS_H
#define _LINUX_SYSFS_H
#include <linux/types.h>

struct attribute {
	const char		*name;
	umode_t			mode;
};

struct attribute_group {
	const char		*name;
	umode_t			(*is_visible)(struct kobject *,
					      struct attribute *, int);
	struct attribute	**attrs;
	struct bin_attribute	**bin_attrs;
};

#define __ATTR_RO(_name) {						\
	.attr	= { .name = __stringify(_name), .mode = S_IRUGO },	\
	.show	= _name##_show,						\
}

static inline int sysfs_create_groups(struct kobject *kobj,
				      const struct attribute_group **groups)
{
	return 0;
}

struct sysfs_ops {
	ssize_t	(*show)(struct kobject *, struct attribute *,char *);
	ssize_t	(*store)(struct kobject *,struct attribute *,const char *, size_t);
};

#endif /* _LINUX_SYSFS_H */
