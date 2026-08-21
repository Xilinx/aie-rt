/******************************************************************************
* Copyright (C) 2026 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_mem_alloc.c
* @{
*
* This file implements a small bounded dynamic memory allocator used as a
* drop-in replacement for the hosted C library's malloc()/calloc()/
* realloc()/free()/aligned_alloc() when XAIE_FEATURE_NO_LIBC_MALLOC is
* defined (the Arm Certified FuSa C library provides malloc() only; there
* is no free(), calloc(), realloc() or aligned_alloc() at all).
*
* First-fit, address-ordered, coalescing free-list over a single static
* arena (XAieHeapArena). Each block is a fixed-size header
* (XAieHeapBlock) followed by its payload; linkage uses u32 byte offsets
* instead of raw pointers, keeping the header size fixed regardless of
* host pointer width.
*
* Every allocation ends up aligned to XAIE_HEAP_ALIGNMENT by
* construction: the arena base is aligned to it, XAIE_HEAP_HDR_SIZE is a
* multiple of it (enforced by a build-time assertion below, since it
* isn't automatic if XAIE_HEAP_ALIGNMENT changes), and every payload
* size is rounded up to a multiple of it. So _XAie_HeapAlignedAlloc()
* can just delegate to _XAie_HeapMalloc() -- no separate bookkeeping.
*
* Traversal cost (find-fit, coalescing) is O(blocks in the arena),
* bounded by XAIE_FUSA_HEAP_SIZE / XAIE_HEAP_ALIGNMENT: deterministic at
* the cost of throughput, an intentional trade-off for a FuSa allocator.
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
#include "xaie_mem_alloc.h"

#ifndef XAIE_FEATURE_LIBC_MALLOC_ENABLE

/***************************** Include Files *********************************/
#include <string.h>

/************************** Constant Definitions ******************************/
#define XAIE_HEAP_NULL ((u32)0xFFFFFFFFU)

/*
 * Largest Size safely narrowed from size_t to the u32 Size field
 * without truncating. A no-op on the real 32-bit target; matters for
 * native host-based testing where size_t may be 64-bit. Unrelated to
 * arena capacity, which _XAie_HeapFindFit() enforces separately.
 */
#define XAIE_HEAP_MAX_REQUEST_SIZE ((size_t)0xFFFFFFFFU)

/**************************** Type Definitions *******************************/
/*
 * Block header: 4 u32 fields, always exactly 16 bytes. Next/Prev are
 * byte offsets into XAieHeapArena of the address-order neighbouring
 * blocks, or XAIE_HEAP_NULL at the ends of the arena.
 */
typedef struct {
	u32 Size;
	u32 Free;
	u32 Next;
	u32 Prev;
} XAieHeapBlock;

#define XAIE_HEAP_HDR_SIZE ((u32)sizeof(XAieHeapBlock))

/*
 * XAIE_HEAP_HDR_SIZE must stay a multiple of XAIE_HEAP_ALIGNMENT, or
 * payload pointers won't actually be aligned. Holds today (16 % 16 ==
 * 0) but isn't automatic if XAIE_HEAP_ALIGNMENT changes, so assert it
 * instead of breaking alignment silently.
 */
_Static_assert((XAIE_HEAP_HDR_SIZE % XAIE_HEAP_ALIGNMENT) == 0U,
		"XAIE_HEAP_HDR_SIZE must be a multiple of XAIE_HEAP_ALIGNMENT, "
		"or payload pointers will not actually be aligned to "
		"XAIE_HEAP_ALIGNMENT bytes");

/************************** Variable Definitions ******************************/
static u8 XAieHeapArena[XAIE_FUSA_HEAP_SIZE]
	__attribute__((aligned(XAIE_HEAP_ALIGNMENT)));
static u32 XAieHeapInitialized;
static XAie_HeapStats XAieHeapStatsInst;

/************************** Function Definitions ******************************/
static XAieHeapBlock *_XAie_HeapBlockAt(u32 Offset)
{
	return (XAieHeapBlock *)(void *)&XAieHeapArena[Offset];
}

static u32 _XAie_HeapOffsetOf(const XAieHeapBlock *Block)
{
	return (u32)((const u8 *)(const void *)Block - XAieHeapArena);
}

static u32 _XAie_HeapAlignUp(u32 Size)
{
	return (Size + (XAIE_HEAP_ALIGNMENT - 1U)) &
		~((u32)XAIE_HEAP_ALIGNMENT - 1U);
}

/*
 * Lazily initializes the arena as a single free block spanning it. Safe
 * to call repeatedly; only the first call has any effect.
 */
static void _XAie_HeapInit(void)
{
	XAieHeapBlock *Root;

	if (XAieHeapInitialized != 0U) {
		return;
	}

	Root = _XAie_HeapBlockAt(0U);
	Root->Size = (u32)sizeof(XAieHeapArena) - XAIE_HEAP_HDR_SIZE;
	Root->Free = 1U;
	Root->Next = XAIE_HEAP_NULL;
	Root->Prev = XAIE_HEAP_NULL;

	XAieHeapStatsInst.TotalSize = Root->Size;
	XAieHeapStatsInst.UsedSize = 0U;
	XAieHeapStatsInst.FreeSize = Root->Size;
	XAieHeapStatsInst.PeakUsedSize = 0U;
	XAieHeapStatsInst.NumAllocs = 0U;
	XAieHeapStatsInst.NumFrees = 0U;
	XAieHeapStatsInst.NumFailures = 0U;

	XAieHeapInitialized = 1U;
}

/* First-fit search over all blocks in address order. */
static u32 _XAie_HeapFindFit(u32 ReqSize)
{
	u32 Offset = 0U;

	while (Offset != XAIE_HEAP_NULL) {
		XAieHeapBlock *Block = _XAie_HeapBlockAt(Offset);

		if ((Block->Free != 0U) && (Block->Size >= ReqSize)) {
			return Offset;
		}
		Offset = Block->Next;
	}

	return XAIE_HEAP_NULL;
}

/*
 * Carves a ReqSize-byte block out of the front of *Block if the leftover
 * remainder is large enough to host another block header plus at least
 * one alignment quantum of payload; otherwise leaves Block as-is (the
 * caller gets a slightly larger block than requested). Does not touch
 * Block->Free; callers set that as appropriate for malloc() vs realloc().
 */
static void _XAie_HeapSplit(XAieHeapBlock *Block, u32 Offset, u32 ReqSize)
{
	u32 Remainder;
	u32 NewOffset;
	XAieHeapBlock *NewBlock;

	Remainder = Block->Size - ReqSize;
	if (Remainder < (XAIE_HEAP_HDR_SIZE + XAIE_HEAP_ALIGNMENT)) {
		return;
	}

	NewOffset = Offset + XAIE_HEAP_HDR_SIZE + ReqSize;
	NewBlock = _XAie_HeapBlockAt(NewOffset);
	NewBlock->Size = Remainder - XAIE_HEAP_HDR_SIZE;
	NewBlock->Free = 1U;
	NewBlock->Next = Block->Next;
	NewBlock->Prev = Offset;

	if (Block->Next != XAIE_HEAP_NULL) {
		_XAie_HeapBlockAt(Block->Next)->Prev = NewOffset;
	}

	Block->Next = NewOffset;
	Block->Size = ReqSize;
}

/*
 * Merges Block with its address-order next neighbour, which must exist
 * and must be free. Block survives (grown); the neighbour ceases to
 * exist as a distinct block. Does not touch Block->Free.
 */
static void _XAie_HeapMergeWithNext(XAieHeapBlock *Block)
{
	XAieHeapBlock *NextBlock = _XAie_HeapBlockAt(Block->Next);

	Block->Size += XAIE_HEAP_HDR_SIZE + NextBlock->Size;
	Block->Next = NextBlock->Next;

	if (NextBlock->Next != XAIE_HEAP_NULL) {
		_XAie_HeapBlockAt(NextBlock->Next)->Prev =
			_XAie_HeapOffsetOf(Block);
	}
}

/* Coalesces a just-freed Block with its free neighbour(s), if any. */
static void _XAie_HeapCoalesce(XAieHeapBlock *Block)
{
	if ((Block->Next != XAIE_HEAP_NULL) &&
			(_XAie_HeapBlockAt(Block->Next)->Free != 0U)) {
		_XAie_HeapMergeWithNext(Block);
	}

	if (Block->Prev != XAIE_HEAP_NULL) {
		XAieHeapBlock *PrevBlock = _XAie_HeapBlockAt(Block->Prev);

		if (PrevBlock->Free != 0U) {
			_XAie_HeapMergeWithNext(PrevBlock);
		}
	}
}

/*****************************************************************************/
/**
*
* Bounded replacement for the hosted C library's malloc(). Returns memory
* aligned to XAIE_HEAP_ALIGNMENT bytes.
*
* @param	Size: Number of bytes requested.
*
* @return	Pointer to a Size-byte buffer, or NULL on failure (zero size,
*		size too large to represent internally, or arena exhausted/
*		too fragmented to satisfy the request).
*
* @note		Internal only.
*
*******************************************************************************/
void *_XAie_HeapMalloc(size_t Size)
{
	u32 ReqSize;
	u32 Offset;
	XAieHeapBlock *Block;

	_XAie_HeapInit();

	if ((Size == 0U) || (Size > XAIE_HEAP_MAX_REQUEST_SIZE)) {
		XAieHeapStatsInst.NumFailures++;
		return NULL;
	}

	ReqSize = _XAie_HeapAlignUp((u32)Size);

	Offset = _XAie_HeapFindFit(ReqSize);
	if (Offset == XAIE_HEAP_NULL) {
		XAieHeapStatsInst.NumFailures++;
		return NULL;
	}

	Block = _XAie_HeapBlockAt(Offset);
	_XAie_HeapSplit(Block, Offset, ReqSize);
	Block->Free = 0U;

	XAieHeapStatsInst.NumAllocs++;
	XAieHeapStatsInst.UsedSize += Block->Size;
	XAieHeapStatsInst.FreeSize -= Block->Size;
	if (XAieHeapStatsInst.UsedSize > XAieHeapStatsInst.PeakUsedSize) {
		XAieHeapStatsInst.PeakUsedSize = XAieHeapStatsInst.UsedSize;
	}

	return (void *)((u8 *)(void *)Block + XAIE_HEAP_HDR_SIZE);
}

/*****************************************************************************/
/**
*
* Bounded replacement for the hosted C library's calloc(). Rejects Num*Size
* overflow before allocating, and zero-fills the returned buffer.
*
* @param	Num: Number of elements.
* @param	Size: Size in bytes of each element.
*
* @return	Pointer to a zero-filled Num*Size-byte buffer, or NULL on
*		failure.
*
* @note		Internal only.
*
*******************************************************************************/
void *_XAie_HeapCalloc(size_t Num, size_t Size)
{
	void *Ptr;

	if ((Num == 0U) || (Size == 0U)) {
		return NULL;
	}

	if (Num > (XAIE_HEAP_MAX_REQUEST_SIZE / Size)) {
		XAieHeapStatsInst.NumFailures++;
		return NULL;
	}

	Ptr = _XAie_HeapMalloc(Num * Size);
	if (Ptr == NULL) {
		return NULL;
	}

	(void)memset(Ptr, 0, Num * Size);

	return Ptr;
}

/*****************************************************************************/
/**
*
* Bounded replacement for the hosted C library's free().
*
* @param	Ptr: Pointer previously returned by _XAie_HeapMalloc(),
*		_XAie_HeapCalloc(), _XAie_HeapRealloc() or
*		_XAie_HeapAlignedAlloc(). NULL is silently ignored.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
void _XAie_HeapFree(void *Ptr)
{
	XAieHeapBlock *Block;

	if (Ptr == NULL) {
		return;
	}

	Block = (XAieHeapBlock *)((u8 *)Ptr - XAIE_HEAP_HDR_SIZE);
	if (Block->Free != 0U) {
		/* Defensively ignore a double free rather than corrupt the
		 * free list. */
		return;
	}

	XAieHeapStatsInst.UsedSize -= Block->Size;
	XAieHeapStatsInst.FreeSize += Block->Size;
	XAieHeapStatsInst.NumFrees++;

	Block->Free = 1U;
	_XAie_HeapCoalesce(Block);
}

/*****************************************************************************/
/**
*
* Bounded replacement for the hosted C library's realloc(). Grows the
* block in place when the immediately following block is free and large
* enough; otherwise allocates a new block, copies the lesser of the old
* and new sizes, and frees the old block. Shrinking (or requesting the
* same size) keeps the existing block as-is (no shrink-split), which
* trades a small amount of internal fragmentation for simplicity -- no
* call site in this codebase relies on realloc() to shrink.
*
* @param	Ptr: Pointer previously returned by one of the _XAie_Heap*()
*		allocation functions, or NULL (in which case this behaves
*		like _XAie_HeapMalloc()).
* @param	Size: New size in bytes. If 0, this behaves like
*		_XAie_HeapFree(Ptr) and returns NULL.
*
* @return	Pointer to a Size-byte buffer (may or may not be Ptr), or
*		NULL on failure (Ptr is left unmodified in that case).
*
* @note		Internal only.
*
*******************************************************************************/
void *_XAie_HeapRealloc(void *Ptr, size_t Size)
{
	XAieHeapBlock *Block;
	u32 ReqSize;
	u32 OldSize;
	void *NewPtr;
	u32 CopySize;

	if (Ptr == NULL) {
		return _XAie_HeapMalloc(Size);
	}
	if (Size == 0U) {
		_XAie_HeapFree(Ptr);
		return NULL;
	}
	if (Size > XAIE_HEAP_MAX_REQUEST_SIZE) {
		XAieHeapStatsInst.NumFailures++;
		return NULL;
	}

	Block = (XAieHeapBlock *)((u8 *)Ptr - XAIE_HEAP_HDR_SIZE);
	ReqSize = _XAie_HeapAlignUp((u32)Size);

	if (Block->Size >= ReqSize) {
		return Ptr;
	}

	if ((Block->Next != XAIE_HEAP_NULL)) {
		XAieHeapBlock *NextBlock = _XAie_HeapBlockAt(Block->Next);

		if ((NextBlock->Free != 0U) &&
				((Block->Size + XAIE_HEAP_HDR_SIZE +
				  NextBlock->Size) >= ReqSize)) {
			u32 Offset = _XAie_HeapOffsetOf(Block);

			OldSize = Block->Size;
			_XAie_HeapMergeWithNext(Block);
			_XAie_HeapSplit(Block, Offset, ReqSize);

			XAieHeapStatsInst.UsedSize += Block->Size - OldSize;
			XAieHeapStatsInst.FreeSize -= Block->Size - OldSize;
			if (XAieHeapStatsInst.UsedSize >
					XAieHeapStatsInst.PeakUsedSize) {
				XAieHeapStatsInst.PeakUsedSize =
					XAieHeapStatsInst.UsedSize;
			}

			return Ptr;
		}
	}

	NewPtr = _XAie_HeapMalloc(Size);
	if (NewPtr == NULL) {
		return NULL;
	}

	CopySize = (Block->Size < ReqSize) ? Block->Size : ReqSize;
	(void)memcpy(NewPtr, Ptr, CopySize);
	_XAie_HeapFree(Ptr);

	return NewPtr;
}

/*****************************************************************************/
/**
*
* Bounded replacement for the hosted C library's aligned_alloc(). Every
* allocation from this allocator is already aligned to
* XAIE_HEAP_ALIGNMENT bytes by construction (see file header comment), so
* this simply validates the requested alignment and delegates to
* _XAie_HeapMalloc().
*
* @param	Align: Requested alignment in bytes. Must be no stricter
*		than XAIE_HEAP_ALIGNMENT (the only value ever requested in
*		this codebase is 16, which matches XAIE_HEAP_ALIGNMENT).
* @param	Size: Number of bytes requested.
*
* @return	Pointer to a Size-byte buffer aligned to Align bytes, or
*		NULL on failure (including an unsupported Align).
*
* @note		Internal only.
*
*******************************************************************************/
void *_XAie_HeapAlignedAlloc(size_t Align, size_t Size)
{
	if ((Align == 0U) || (Align > (size_t)XAIE_HEAP_ALIGNMENT)) {
		XAieHeapStatsInst.NumFailures++;
		return NULL;
	}

	return _XAie_HeapMalloc(Size);
}

/*****************************************************************************/
/**
*
* Returns a snapshot of allocator usage statistics. Primarily useful
* during integration testing to measure actual peak usage and right-size
* XAIE_FUSA_HEAP_SIZE, and as supporting evidence for a bounded-memory
* argument during FuSa certification.
*
* @param	Stats: Output parameter; NULL is silently ignored.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
void _XAie_HeapGetStats(XAie_HeapStats *Stats)
{
	if (Stats == NULL) {
		return;
	}

	_XAie_HeapInit();
	*Stats = XAieHeapStatsInst;
}

#endif /* !XAIE_FEATURE_LIBC_MALLOC_ENABLE */
/** @} */
