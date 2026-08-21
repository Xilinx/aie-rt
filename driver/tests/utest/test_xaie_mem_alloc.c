/******************************************************************************
* Copyright (C) 2026 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*
 * Standalone native sanity test for the FuSa arena allocator
 * (xaie_mem_alloc.h/.c). This is intentionally independent of the rest of
 * the driver (no hardware config, no CppUTest) so it can be compiled and
 * run directly on the host with a plain C compiler, e.g.:
 *
 *   gcc -std=c11 -Wall -Wextra -DXAIE_FEATURE_NO_LIBC_MALLOC \
 *       -DXAIE_FUSA_HEAP_SIZE=8192 \
 *       -I../../src/common -I../../src/global \
 *       test_xaie_mem_alloc.c ../../src/common/xaie_mem_alloc.c \
 *       -o test_xaie_mem_alloc
 *   ./test_xaie_mem_alloc
 *
 * It exercises malloc/calloc/realloc/free/aligned_alloc semantics,
 * XAIE_HEAP_ALIGNMENT-byte alignment, coalescing on free, and behavior at
 * arena exhaustion (OOM), all against the actual production allocator
 * source file compiled with XAIE_FEATURE_NO_LIBC_MALLOC (i.e. the exact
 * translation unit used on a FuSa target), not a reimplementation.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "xaie_mem_alloc.h"

#ifdef XAIE_FEATURE_LIBC_MALLOC_ENABLE
#error "This test must be built with -DXAIE_FEATURE_NO_LIBC_MALLOC so it " \
       "exercises the custom arena allocator, not the passthrough macros."
#endif

static int NumFailed;
static int NumChecked;

#define CHECK(cond) do { \
	NumChecked++; \
	if (!(cond)) { \
		printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
		NumFailed++; \
	} \
} while (0)

static int IsAligned(const void *Ptr)
{
	return ((uintptr_t)Ptr % XAIE_HEAP_ALIGNMENT) == 0U;
}

static void TestBasicMallocFree(void)
{
	void *P1 = XAIE_MALLOC(64U);
	void *P2 = XAIE_MALLOC(128U);

	CHECK(P1 != NULL);
	CHECK(P2 != NULL);
	CHECK(P1 != P2);
	CHECK(IsAligned(P1));
	CHECK(IsAligned(P2));

	memset(P1, 0xAB, 64U);
	memset(P2, 0xCD, 128U);
	CHECK(((unsigned char *)P1)[0] == 0xABU);
	CHECK(((unsigned char *)P1)[63] == 0xABU);
	CHECK(((unsigned char *)P2)[127] == 0xCDU);

	XAIE_FREE(P1);
	XAIE_FREE(P2);
}

static void TestAlignmentAcrossSizes(void)
{
	static const size_t Sizes[] = {1U, 2U, 3U, 7U, 15U, 16U, 17U, 31U,
		33U, 63U, 100U, 255U, 1000U};
	size_t i;

	for (i = 0U; i < (sizeof(Sizes) / sizeof(Sizes[0])); i++) {
		void *Ptr = XAIE_MALLOC(Sizes[i]);

		CHECK(Ptr != NULL);
		CHECK(IsAligned(Ptr));
		memset(Ptr, 0x5A, Sizes[i]);
		XAIE_FREE(Ptr);
	}
}

static void TestCallocZeroesMemory(void)
{
	unsigned char *Buf = (unsigned char *)XAIE_CALLOC(37U, sizeof(*Buf));
	size_t i;

	CHECK(Buf != NULL);
	CHECK(IsAligned(Buf));
	for (i = 0U; i < 37U; i++) {
		CHECK(Buf[i] == 0U);
	}
	XAIE_FREE(Buf);
}

static void TestCallocOverflowRejected(void)
{
	/* Num * Size overflows size_t: must fail cleanly, not wrap and
	 * "succeed" with a too-small buffer. */
	void *Ptr = XAIE_CALLOC((size_t)-1, 2U);

	CHECK(Ptr == NULL);
}

static void TestReallocGrowPreservesData(void)
{
	u32 *Buf = (u32 *)XAIE_MALLOC(4U * sizeof(u32));
	u32 *Grown;
	int i;

	CHECK(Buf != NULL);
	for (i = 0; i < 4; i++) {
		Buf[i] = (u32)(1000 + i);
	}

	Grown = (u32 *)XAIE_REALLOC(Buf, 64U * sizeof(u32));
	CHECK(Grown != NULL);
	CHECK(IsAligned(Grown));
	for (i = 0; i < 4; i++) {
		CHECK(Grown[i] == (u32)(1000 + i));
	}
	/* The whole new region must be writable. */
	for (i = 0; i < 64; i++) {
		Grown[i] = (u32)i;
	}
	CHECK(Grown[63] == 63U);

	XAIE_FREE(Grown);
}

static void TestReallocNullBehavesLikeMalloc(void)
{
	void *Ptr = XAIE_REALLOC(NULL, 32U);

	CHECK(Ptr != NULL);
	CHECK(IsAligned(Ptr));
	XAIE_FREE(Ptr);
}

static void TestReallocZeroBehavesLikeFree(void)
{
	void *Ptr = XAIE_MALLOC(32U);
	void *Result;

	CHECK(Ptr != NULL);
	Result = XAIE_REALLOC(Ptr, 0U);
	CHECK(Result == NULL);
	/* Ptr must now be back on the free list; a fresh malloc of the
	 * same size should be able to reuse that space (checked indirectly
	 * via the coalescing/reuse test below). */
}

static void TestFreeNullIsNoop(void)
{
	XAIE_FREE(NULL);
	CHECK(1); /* Reaching here without crashing is the test. */
}

static void TestDoubleFreeIsIgnoredDefensively(void)
{
	void *Ptr = XAIE_MALLOC(16U);

	CHECK(Ptr != NULL);
	XAIE_FREE(Ptr);
	XAIE_FREE(Ptr); /* Must not corrupt the free list or crash. */

	/* Allocator must still be usable afterwards. */
	void *Sanity = XAIE_MALLOC(16U);
	CHECK(Sanity != NULL);
	XAIE_FREE(Sanity);
}

static void TestAlignedAllocSupportedAndUnsupported(void)
{
	/* XAIE_ALIGNED_ALLOC() rejects any non-matching literal at compile
	 * time, so call the underlying function directly to test its
	 * runtime contract instead. */
	void *Ptr16 = XAIE_ALIGNED_ALLOC(16U, 100U);
	void *PtrDefault = _XAie_HeapAlignedAlloc(1U, 100U);
	void *Ptr32 = _XAie_HeapAlignedAlloc(32U, 100U);

	CHECK(Ptr16 != NULL);
	CHECK(IsAligned(Ptr16));
	CHECK(PtrDefault != NULL);
	CHECK(IsAligned(PtrDefault));
	/* Stricter-than-supported alignment must fail cleanly rather than
	 * silently return an insufficiently-aligned buffer. */
	CHECK(Ptr32 == NULL);

	XAIE_FREE(Ptr16);
	XAIE_FREE(PtrDefault);
}

static void TestCoalescingReclaimsFullBlock(void)
{
	XAie_HeapStats Before;
	XAie_HeapStats After;
	void *A;
	void *B;
	void *C;
	void *Big;

	_XAie_HeapGetStats(&Before);

	A = XAIE_MALLOC(256U);
	B = XAIE_MALLOC(256U);
	C = XAIE_MALLOC(256U);
	CHECK(A != NULL && B != NULL && C != NULL);

	/* Free out of address order (middle, then front, then back) to
	 * exercise merge-with-next and merge-with-prev. */
	XAIE_FREE(B);
	XAIE_FREE(A);
	XAIE_FREE(C);

	_XAie_HeapGetStats(&After);
	CHECK(After.UsedSize == Before.UsedSize);
	CHECK(After.FreeSize == Before.FreeSize);

	/* If A/B/C fully coalesced back into one block (rather than staying
	 * as 3 separate free blocks), we can allocate a single block close
	 * to the combined size again. */
	Big = XAIE_MALLOC(700U);
	CHECK(Big != NULL);
	XAIE_FREE(Big);
}

static void TestArenaExhaustionReturnsNullWithoutCrashing(void)
{
	XAie_HeapStats StatsBefore;
	XAie_HeapStats StatsAfter;
	/* Sized from XAIE_FUSA_HEAP_SIZE so this stays a valid OOM check
	 * regardless of the configured default. */
	void *Ptrs[(XAIE_FUSA_HEAP_SIZE / 64U) + 16U];
	size_t Count = 0U;
	size_t i;

	_XAie_HeapGetStats(&StatsBefore);

	while (Count < (sizeof(Ptrs) / sizeof(Ptrs[0]))) {
		void *Ptr = XAIE_MALLOC(64U);

		if (Ptr == NULL) {
			break;
		}
		Ptrs[Count] = Ptr;
		Count++;
	}

	/* Arena is bounded, so this loop must terminate via OOM well before
	 * exhausting the Ptrs[] array itself. */
	CHECK(Count < (sizeof(Ptrs) / sizeof(Ptrs[0])));
	CHECK(Count > 0U);

	{
		void *ShouldFail = XAIE_MALLOC(64U);

		CHECK(ShouldFail == NULL);
	}

	_XAie_HeapGetStats(&StatsAfter);
	CHECK(StatsAfter.NumFailures > StatsBefore.NumFailures);

	for (i = 0U; i < Count; i++) {
		XAIE_FREE(Ptrs[i]);
	}

	/* Freeing everything must return the arena to fully reusable
	 * (coalesced) state. */
	{
		void *Recheck = XAIE_MALLOC(64U);

		CHECK(Recheck != NULL);
		XAIE_FREE(Recheck);
	}
}

static void TestStatsPeakTracking(void)
{
	XAie_HeapStats Stats;
	void *P1 = XAIE_MALLOC(500U);
	void *P2 = XAIE_MALLOC(500U);

	_XAie_HeapGetStats(&Stats);
	CHECK(Stats.PeakUsedSize >= 1000U);
	CHECK(Stats.TotalSize == (u32)XAIE_FUSA_HEAP_SIZE - 16U);

	XAIE_FREE(P1);
	XAIE_FREE(P2);
}

int main(void)
{
	TestBasicMallocFree();
	TestAlignmentAcrossSizes();
	TestCallocZeroesMemory();
	TestCallocOverflowRejected();
	TestReallocGrowPreservesData();
	TestReallocNullBehavesLikeMalloc();
	TestReallocZeroBehavesLikeFree();
	TestFreeNullIsNoop();
	TestDoubleFreeIsIgnoredDefensively();
	TestAlignedAllocSupportedAndUnsupported();
	TestCoalescingReclaimsFullBlock();
	TestStatsPeakTracking();
	TestArenaExhaustionReturnsNullWithoutCrashing();

	if (NumFailed == 0) {
		printf("ALL TESTS PASSED (%d checks)\n", NumChecked);
		return 0;
	}

	printf("%d of %d CHECK(S) FAILED\n", NumFailed, NumChecked);
	return 1;
}
