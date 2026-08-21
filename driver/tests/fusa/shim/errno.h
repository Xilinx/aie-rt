/* Verify-build shim, not part of the driver source tree.
 *
 * Mirrors the real Arm FuSa <errno.h>: EDOM/ERANGE only (values 1/2, not
 * POSIX's 33/34), used when the real package is unavailable. EINVAL/
 * ENOMEM/EEXIST are deliberately absent here too -- btree4.h supplies
 * its own fallbacks, matching what the real library omits.
 */
#ifndef _FUSA_VERIFY_ERRNO_H
#define _FUSA_VERIFY_ERRNO_H

extern int errno;

#define EDOM    1
#define ERANGE  2

#endif /* _FUSA_VERIFY_ERRNO_H */
