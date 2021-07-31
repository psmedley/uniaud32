/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * fwnode.h - Firmware device node object handle type definition.
 *
 * Copyright (C) 2015, Intel Corporation
 * Author: Rafael J. Wysocki <rafael.j.wysocki@intel.com>
 */

#ifndef _LINUX_FWNODE_H_
#define _LINUX_FWNODE_H_

#include <linux/types.h>

struct fwnode_operations;
struct device;

struct fwnode_handle {
	struct fwnode_handle *secondary;
	const struct fwnode_operations *ops;
	struct device *dev;
};

/**
 * struct fwnode_endpoint - Fwnode graph endpoint
 * @port: Port number
 * @id: Endpoint id
 * @local_fwnode: reference to the related fwnode
 */
struct fwnode_endpoint {
	unsigned int port;
	unsigned int id;
	const struct fwnode_handle *local_fwnode;
};

#endif
