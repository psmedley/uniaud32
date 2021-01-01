/* $Id: uio.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_UIO_H
#define _LINUX_UIO_H

enum {
	ITER_IOVEC = 0,
	ITER_KVEC = 2,
	ITER_BVEC = 4,
};

struct iovec {
        char    *iov_base;      /* Base address. */
#ifdef __32BIT__
        size_t   iov_len;       /* Length. */
#else
        long     iov_len;       /* Length. */
#endif
};

struct iov_iter {
	int type;
	size_t iov_offset;
	size_t count;
	union {
		const struct iovec *iov;
		const struct kvec *kvec;
		const struct bio_vec *bvec;
	};
	unsigned long nr_segs;
};

static inline bool iter_is_iovec(const struct iov_iter *i)
{
	return !(i->type & (ITER_BVEC | ITER_KVEC));
}

#endif /* _LINUX_UIO_H */
