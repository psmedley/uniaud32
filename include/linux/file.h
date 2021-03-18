/*
 * Wrapper functions for accessing the file_struct fd array.
 */

#ifndef __LINUX_FILE_H
#define __LINUX_FILE_H

#include <linux/types.h>

struct fd {
	struct file *file;
	int need_put;
};

extern void _fput(struct file *);

/*
 * Check whether the specified task has the fd open. Since the task
 * may not have a files_struct, we must test for p->files != NULL.
 */
extern struct file * fcheck_task(struct task_struct *p, unsigned int fd);

/*
 * Check whether the specified fd has an open file.
 */
extern inline struct file * fcheck(unsigned int fd);

extern inline struct file * frip(unsigned int fd);

extern inline struct file * fget(unsigned int fd);

/*
 * 23/12/1998 Marcin Dalecki <dalecki@cs.net.pl>: 
 * 
 * Since those functions where calling other functions, it was compleatly 
 * bogous to make them all "extern inline".
 *
 * The removal of this pseudo optimization saved me scandaleous:
 *
 * 		3756 (i386 arch) 
 *
 * precious bytes from my kernel, even without counting all the code compiled
 * as module!
 *
 * I suspect there are many other similar "optimizations" across the
 * kernel...
 */
extern inline void fput(struct file * file);
extern void put_filp(struct file *);

/*
 * Install a file pointer in the fd array.  
 *
 * The VFS is full of places where we drop the files lock between
 * setting the open_fds bitmap and installing the file in the file
 * array.  At any such point, we are vulnerable to a dup2() race
 * installing a file in the array before us.  We need to detect this and
 * fput() the struct file we are about to overwrite in this case.
 */

extern inline void fd_install(unsigned int fd, struct file * file);

#endif /* __LINUX_FILE_H */
