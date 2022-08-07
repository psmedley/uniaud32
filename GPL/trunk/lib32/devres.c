/*
 * drivers/base/devres.c - device resource management
 *
 * Copyright (c) 2006  SUSE Linux Products GmbH
 * Copyright (c) 2006  Tejun Heo <teheo@suse.de>
 *
 * This file is released under the GPLv2.
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/errno.h>

struct devres_node {
	struct list_head		entry;
	dr_release_t			release;
#ifdef CONFIG_DEBUG_DEVRES
	const char			*name;
	size_t				size;
#endif
};

struct devres {
	struct devres_node		node;
	/* -- 3 pointers */
	unsigned long long		data[];	/* guarantee ull alignment */
};

struct devres_group {
	struct devres_node		node[2];
	void				*id;
	int				color;
	/* -- 8 pointers */
};

#define devres_log(dev, node, op)	do {} while (0)

/*
 * Release functions for devres group.  These callbacks are used only
 * for identification.
 */
static void group_open_release(struct device *dev, void *res)
{
	/* noop */
}

static void group_close_release(struct device *dev, void *res)
{
	/* noop */
}

static struct devres_group * node_to_group(struct devres_node *node)
{
	if (node->release == &group_open_release)
		return container_of(node, struct devres_group, node[0]);
	if (node->release == &group_close_release)
		return container_of(node, struct devres_group, node[1]);
	return NULL;
}

static inline struct devres * alloc_dr(dr_release_t release,
						size_t size, gfp_t gfp, int nid)
{
	size_t tot_size = sizeof(struct devres) + size;
	struct devres *dr;

	dr = kmalloc_node_track_caller(tot_size, gfp, nid);

	memset(dr, 0, offsetof(struct devres, data));

	INIT_LIST_HEAD(&dr->node.entry);
	dr->node.release = release;
	return dr;
}

#define devres_log(dev, node, op)	do {} while (0)

static void add_dr(struct device *dev, struct devres_node *node)
{
	devres_log(dev, node, "ADD");
	BUG_ON(!list_empty(&node->entry));
//#ifndef TARGET_OS2
	/* Traps here on OS/2 */
	list_add_tail(&node->entry, &dev->devres_head);
//#endif
}

/**
 * devres_add - Register device resource
 * @dev: Device to add resource to
 * @res: Resource to register
 *
 * Register devres @res to @dev.  @res should have been allocated
 * using devres_alloc().  On driver detach, the associated release
 * function will be invoked and devres will be freed automatically.
 */
void devres_add(struct device *dev, void *res)
{
	/* Traps here on OS/2 */
	struct devres *dr = container_of(res, struct devres, data);
	unsigned long flags;
	spin_lock_irqsave(&dev->devres_lock, flags);
	add_dr(dev, &dr->node);
	spin_unlock_irqrestore(&dev->devres_lock, flags);
}

/**
 * devres_alloc - Allocate device resource data
 * @release: Release function devres will be associated with
 * @size: Allocation size
 * @gfp: Allocation flags
 * @nid: NUMA node
 *
 * Allocate devres of @size bytes.  The allocated area is zeroed, then
 * associated with @release.  The returned pointer can be passed to
 * other devres_*() functions.
 *
 * RETURNS:
 * Pointer to allocated devres on success, NULL on failure.
 */
void * devres_alloc_node(dr_release_t release, size_t size, gfp_t gfp, int nid)
{
	struct devres *dr;

	dr = alloc_dr(release, size, gfp | __GFP_ZERO, nid);
	return dr->data;
}

/**
 * devres_free - Free device resource data
 * @res: Pointer to devres data to free
 *
 * Free devres created with devres_alloc().
 */
void devres_free(void *res)
{
	if (res) {
		struct devres *dr = container_of(res, struct devres, data);

		BUG_ON(!list_empty(&dr->node.entry));
		kfree(dr);
	}
}

static int remove_nodes(struct device *dev,
			struct list_head *first, struct list_head *end,
			struct list_head *todo)
{
	int cnt = 0, nr_groups = 0;
	struct list_head *cur;

	/* First pass - move normal devres entries to @todo and clear
	 * devres_group colors.
	 */
	cur = first;
	while (cur != end) {
		struct devres_node *node;
		struct devres_group *grp;

		node = list_entry(cur, struct devres_node, entry);
		cur = cur->next;

		grp = node_to_group(node);
		if (grp) {
			/* clear color of group markers in the first pass */
			grp->color = 0;
			nr_groups++;
		} else {
			/* regular devres entry */
			if (&node->entry == first)
				first = first->next;
			list_move_tail(&node->entry, todo);
			cnt++;
		}
	}

	if (!nr_groups)
		return cnt;

	/* Second pass - Scan groups and color them.  A group gets
	 * color value of two iff the group is wholly contained in
	 * [cur, end).  That is, for a closed group, both opening and
	 * closing markers should be in the range, while just the
	 * opening marker is enough for an open group.
	 */
	cur = first;
	while (cur != end) {
		struct devres_node *node;
		struct devres_group *grp;

		node = list_entry(cur, struct devres_node, entry);
		cur = cur->next;

		grp = node_to_group(node);
		BUG_ON(!grp || list_empty(&grp->node[0].entry));

		grp->color++;
		if (list_empty(&grp->node[1].entry))
			grp->color++;

		BUG_ON(grp->color <= 0 || grp->color > 2);
		if (grp->color == 2) {
			/* No need to update cur or end.  The removed
			 * nodes are always before both.
			 */
			list_move_tail(&grp->node[0].entry, todo);
			list_del_init(&grp->node[1].entry);
		}
	}

	return cnt;
}

static int release_nodes(struct device *dev, struct list_head *first,
			 struct list_head *end, unsigned long flags)
{
//	LIST_HEAD(todo);
	struct list_head todo;

	int cnt;
	struct devres *dr, *tmp;

	cnt = remove_nodes(dev, first, end, &todo);

	spin_unlock_irqrestore(&dev->devres_lock, flags);

	/* Release.  Note that both devres and devres_group are
	 * handled as devres in the following loop.  This is safe.
	 */
	list_for_each_entry_safe_reverse(dr, tmp, &todo, node.entry, struct devres) {
		devres_log(dev, &dr->node, "REL");
		dr->node.release(dev, dr->data);
		kfree(dr);
	}

	return cnt;
}

/**
 * devres_release_all - Release all managed resources
 * @dev: Device to release resources for
 *
 * Release all resources associated with @dev.  This function is
 * called on driver detach.
 */
int devres_release_all(struct device *dev)
{
	unsigned long flags;

	/* Looks like an uninitialized device structure */
	if (WARN_ON(dev->devres_head.next == NULL))
		return -ENODEV;
	spin_lock_irqsave(&dev->devres_lock, flags);
	return release_nodes(dev, dev->devres_head.next, &dev->devres_head,
			     flags);
}

static struct devres *find_dr(struct device *dev, dr_release_t release,
			      dr_match_t match, void *match_data)
{
	struct devres_node *node;

	list_for_each_entry_reverse(node, &dev->devres_head, entry, struct devres_node) {
		struct devres *dr = container_of(node, struct devres, node);

		if (node->release != release)
			continue;
		if (match && !match(dev, dr->data, match_data))
			continue;
		return dr;
	}

	return NULL;
}

/**
 * devres_find - Find device resource
 * @dev: Device to lookup resource from
 * @release: Look for resources associated with this release function
 * @match: Match function (optional)
 * @match_data: Data for the match function
 *
 * Find the latest devres of @dev which is associated with @release
 * and for which @match returns 1.  If @match is NULL, it's considered
 * to match all.
 *
 * RETURNS:
 * Pointer to found devres, NULL if not found.
 */
void * devres_find(struct device *dev, dr_release_t release,
		   dr_match_t match, void *match_data)
{
	struct devres *dr;
	unsigned long flags;

	spin_lock_irqsave(&dev->devres_lock, flags);
	dr = find_dr(dev, release, match, match_data);
	spin_unlock_irqrestore(&dev->devres_lock, flags);

	if (dr)
		return dr->data;
	return NULL;
}

/**
 * devm_add_action() - add a custom action to list of managed resources
 * @dev: Device that owns the action
 * @action: Function that should be called
 * @data: Pointer to data passed to @action implementation
 *
 * This adds a custom action to the list of managed resources so that
 * it gets executed as part of standard resource unwinding.
 */
int devm_add_action(struct device *dev, void (*action)(void *), void *data)
{
	return 0;
}

/**
 * devm_remove_action() - removes previously added custom action
 * @dev: Device that owns the action
 * @action: Function implementing the action
 * @data: Pointer to data passed to @action implementation
 *
 * Removes instance of @action previously added by devm_add_action().
 * Both action and data should match one of the existing entries.
 */
void devm_remove_action(struct device *dev, void (*action)(void *), void *data)
{
}
