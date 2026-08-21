/******************************************************************************
* Copyright (C) 2026 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_mem_alloc.h
* @{
*
* This file declares a small bounded dynamic memory allocator used as a
* drop-in replacement for the hosted C library's malloc()/calloc()/
* realloc()/free()/aligned_alloc() on targets whose C library does not
* provide them (see XAIE_FEATURE_NO_LIBC_MALLOC in xaie_feature_config.h).
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  ----------  --------------------------------------------
* 1.0   yosong  08/05/2026  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_MEM_ALLOC_H
#define XAIE_MEM_ALLOC_H

/***************************** Include Files *********************************/
#include <stddef.h>
#include "xaie_feature_config.h"
#include "xaiegbl_defs.h"

#ifdef XAIE_FEATURE_LIBC_MALLOC_ENABLE
#include <stdlib.h>
#endif

/************************** Constant Definitions ******************************/
/*
 * Total size in bytes of the static arena backing _XAie_Heap*() when
 * XAIE_FEATURE_NO_LIBC_MALLOC is defined. There is no universally safe
 * default: usage scales with enabled features (transaction mode,
 * routing, ...) and the integrator's memory map, neither known here.
 * Oversizing fails loudly at link time (.bss won't fit); undersizing
 * fails silently at runtime (NumFailures) unless the worst case is
 * actually tested -- default larger for that reason. Override with
 * -DXAIE_FUSA_HEAP_SIZE=<bytes> and use _XAie_HeapGetStats() to
 * right-size for your target before shipping. Note transaction-mode
 * command count is caller-controlled and not bounded by this driver.
 */
#ifndef XAIE_FUSA_HEAP_SIZE
#define XAIE_FUSA_HEAP_SIZE (1048576U)
#endif

/* All allocations returned by the arena allocator are aligned to this many
 * bytes. This matches the only alignment value ever requested via
 * aligned_alloc() in this codebase (16), so a single allocation path can
 * serve both XAIE_MALLOC()/XAIE_CALLOC()/XAIE_REALLOC() and
 * XAIE_ALIGNED_ALLOC() without separate bookkeeping. */
#define XAIE_HEAP_ALIGNMENT (16U)

/**************************** Type Definitions *******************************/
/*
 * Snapshot of allocator usage, primarily useful as evidence for arguing a
 * bounded memory footprint and for right-sizing XAIE_FUSA_HEAP_SIZE.
 */
typedef struct {
	u32 TotalSize;
	u32 UsedSize;
	u32 FreeSize;
	u32 PeakUsedSize;
	u32 NumAllocs;
	u32 NumFrees;
	u32 NumFailures;
} XAie_HeapStats;

#ifdef XAIE_FEATURE_LIBC_MALLOC_ENABLE

/************************** Macro Definitions **********************************/
#define XAIE_MALLOC(Size)			malloc(Size)
#define XAIE_CALLOC(Num, Size)			calloc(Num, Size)
#define XAIE_REALLOC(Ptr, Size)			realloc(Ptr, Size)
#define XAIE_FREE(Ptr)				free(Ptr)
#define XAIE_ALIGNED_ALLOC(Align, Size)		aligned_alloc(Align, Size)

#else /* !XAIE_FEATURE_LIBC_MALLOC_ENABLE */

/************************** Function Prototypes *******************************/
void *_XAie_HeapMalloc(size_t Size);
void *_XAie_HeapCalloc(size_t Num, size_t Size);
void *_XAie_HeapRealloc(void *Ptr, size_t Size);
void _XAie_HeapFree(void *Ptr);
void *_XAie_HeapAlignedAlloc(size_t Align, size_t Size);
void _XAie_HeapGetStats(XAie_HeapStats *Stats);

/************************** Macro Definitions **********************************/
#define XAIE_MALLOC(Size)			_XAie_HeapMalloc(Size)
#define XAIE_CALLOC(Num, Size)			_XAie_HeapCalloc(Num, Size)
#define XAIE_REALLOC(Ptr, Size)			_XAie_HeapRealloc(Ptr, Size)
#define XAIE_FREE(Ptr)				_XAie_HeapFree(Ptr)

/*
 * Only supports exactly XAIE_HEAP_ALIGNMENT: every arena block is
 * already aligned to that by construction, with no per-block padding
 * bookkeeping for anything else -- not a general aligned_alloc(). All
 * real call sites pass a literal 16, so reject any other literal at
 * compile time instead of failing at runtime. To need a different
 * alignment, raise XAIE_HEAP_ALIGNMENT (and XAieHeapBlock's size to
 * match) for the whole arena rather than adding per-call bookkeeping.
 */
#define XAIE_ALIGNED_ALLOC(Align, Size)				\
	(__extension__ ({						\
		_Static_assert((Align) == XAIE_HEAP_ALIGNMENT,		\
			"XAIE_ALIGNED_ALLOC: this allocator only "	\
			"supports XAIE_HEAP_ALIGNMENT-byte alignment; " \
			"see the XAIE_ALIGNED_ALLOC comment in "	\
			"xaie_mem_alloc.h if a different alignment is " \
			"genuinely needed");				\
		_XAie_HeapAlignedAlloc((Align), (Size));		\
	}))

#endif /* XAIE_FEATURE_LIBC_MALLOC_ENABLE */

#endif /* XAIE_MEM_ALLOC_H */
/** @} */
