/* $Id: poll.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_POLL_H
#define _LINUX_POLL_H

#include <asm/poll.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/fs.h>

#ifdef __KERNEL__

struct poll_table_entry {
	struct file * filp;
	wait_queue_t wait;
	wait_queue_head_t * wait_address;
};

typedef struct poll_table_struct {
	struct poll_table_struct * next;
	unsigned int nr;
	struct poll_table_entry * entry;
} poll_table;

#define __MAX_POLL_TABLE_ENTRIES ((PAGE_SIZE - sizeof (poll_table)) / sizeof (struct poll_table_entry))

void init_waitqueue_head(wait_queue_head_t *q);

extern void __pollwait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p);

extern void poll_wait(struct file * filp, wait_queue_head_t * wait_address, poll_table *p);

/*
 * For the kernel fd_set we use a fixed set-size for allocation purposes.
 * This set-size doesn't necessarily bear any relation to the size the user
 * uses, but should preferably obviously be larger than any possible user
 * size (NR_OPEN bits).
 *
 * We need 6 bitmaps (in/out/ex for both incoming and outgoing), and we
 * allocate one page for all the bitmaps. Thus we have 8*PAGE_SIZE bits,
 * to be divided by 6. And we'd better make sure we round to a full
 * long-word (in fact, we'll round to 64 bytes).
 */


#define KFDS_64BLOCK ((PAGE_SIZE/(6*64))*64)
#define KFDS_NR (KFDS_64BLOCK*8 > NR_OPEN ? NR_OPEN : KFDS_64BLOCK*8)
typedef unsigned long kernel_fd_set[KFDS_NR/__NFDBITS];

/*
 * Scaleable version of the fd_set.
 */

typedef struct {
	unsigned long *in, *out, *ex;
	unsigned long *res_in, *res_out, *res_ex;
} fd_set_bits;

/*
 * How many longwords for "nr" bits?
 */
#define FDS_BITPERLONG	(8*sizeof(long))
#define FDS_LONGS(nr)	(((nr)+FDS_BITPERLONG-1)/FDS_BITPERLONG)
#define FDS_BYTES(nr)	(FDS_LONGS(nr)*sizeof(long))

/*
 * We do a VERIFY_WRITE here even though we are only reading this time:
 * we'll write to it eventually..
 *
 * Use "unsigned long" accesses to let user-mode fd_set's be long-aligned.
 */
static int get_fd_set(unsigned long nr, void *ufdset, unsigned long *fdset);

static void set_fd_set(unsigned long nr, void *ufdset, unsigned long *fdset);

static void zero_fd_set(unsigned long nr, unsigned long *fdset);

extern int do_select(int n, fd_set_bits *fds, long *timeout);

#endif /* KERNEL */

#endif /* _LINUX_POLL_H */
