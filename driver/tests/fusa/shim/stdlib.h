/******************************************************************************
* Copyright (C) 2026 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*
 * Default-mode-only shim (see verify_fusa_build.sh's DEFAULT_INC_FLAGS).
 * armclang's own bundled bare-metal <stdlib.h> lacks aligned_alloc() (a
 * C11 addition many embedded libcs skip); a real hosted target (glibc)
 * has it. Forward to the real header and add just that declaration.
 */
#ifndef XAIE_FUSA_TEST_SHIM_STDLIB_H
#define XAIE_FUSA_TEST_SHIM_STDLIB_H

#include_next <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

void *aligned_alloc(size_t Alignment, size_t Size);

#ifdef __cplusplus
}
#endif

#endif /* XAIE_FUSA_TEST_SHIM_STDLIB_H */
