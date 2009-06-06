/* $Id: uio.h,v 1.1.1.1 2003/07/02 13:57:00 eleph Exp $ */

#ifndef _LINUX_UIO_H
#define _LINUX_UIO_H
struct iovec {
        char    *iov_base;      /* Base address. */
#ifdef __32BIT__
        size_t   iov_len;       /* Length. */
#else
        long     iov_len;       /* Length. */
#endif
};

#endif /* _LINUX_UIO_H */
