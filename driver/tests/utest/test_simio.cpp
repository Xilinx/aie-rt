/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#ifndef FILES_MISSING

#include "CppUTest/TestHarness.h"
#include <hw_config.h>

extern "C" {
#include "xaiengine/btree4.h"
#include "xaiengine/xaiegbl.h"
}

struct XAie_DevMem {
	ssize_t Size;
	struct XAie_DevMem *Next;
	struct XAie_DevMem *Prev;
	uint8_t Free : 1;
	XAie_MemInst MemInst;
};

typedef struct {
	u64 BaseAddr;
	u64 NpiBaseAddr;
	XAie_DevInst *DevInst;
	const XAie_CoreMod *OrigCoreMod;
	XAie_CoreMod CoreModOverride;
	struct XAie_DevMem DevMem;
	struct btree4 btree;
} XAie_SimIO;

extern "C" {
AieRC XAie_SimMemSyncForCPUVAddr(XAie_DevInst *DevInst, void *VAddr,
					uint64_t Size);
AieRC XAie_SimMemSyncForDevVAddr(XAie_DevInst *DevInst, void *VAddr,
					uint64_t Size);
}
static XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
		XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
		XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
		XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
		XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

static XAie_InstDeclare(DevInst, &ConfigPtr);

TEST_GROUP(SimIO)
{
	void setup()
	{
		AieRC RC;

		RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
		CHECK_EQUAL(XAIE_OK, RC);

		RC = XAie_PartitionInitialize(&(DevInst), NULL);
		CHECK_EQUAL(XAIE_OK, RC);
	}
	void teardown()
	{
		AieRC RC;

		RC = XAie_PartitionTeardown(&(DevInst));
		CHECK_EQUAL(XAIE_OK, RC);

		XAie_Finish(&DevInst);
	}
};

TEST(SimIO, MemAlloc)
{
#ifdef __AIESIM__
#define SIZE 256
#define NUM	512

	XAie_MemInst *MemInst[NUM];
	XAie_MemInst *Node;
	XAie_MemInst Mem;
	int i;
	XAie_SimIO *IOInst = (XAie_SimIO *)DevInst.IOInst;
	AieRC RC;

	for (i = 0; i < NUM; i++) {
		MemInst[i] = XAie_MemAllocate(&DevInst, SIZE,
					      XAIE_MEM_NONCACHEABLE);
		CHECK(MemInst[i] != NULL);

		Mem.VAddr = MemInst[i]->VAddr;
		Mem.Size = MemInst[i]->Size;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK_EQUAL(MemInst[i], Node);

		Mem.VAddr = (char *)MemInst[i]->VAddr + 64;
		Mem.Size = MemInst[i]->Size;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK_EQUAL(MemInst[i], Node);

		Mem.VAddr = (char *)MemInst[i]->VAddr + 128;
		Mem.Size = 64;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK_EQUAL(MemInst[i], Node);

		Mem.VAddr = (char *)MemInst[i]->VAddr + 1024;
		Mem.Size = 64;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK(MemInst[i] != Node);

		Mem.VAddr = (void *)1024;
		Mem.Size = 64;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK(MemInst[i] != Node);

		Mem.VAddr = NULL;
		Mem.Size = 0xffff;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK(MemInst[i] != Node);

		Mem.VAddr = (char *)MemInst[i]->VAddr + MemInst[i]->Size;
		Mem.Size = MemInst[i]->Size;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK(MemInst[i] != Node);

		Mem.VAddr = (char *)MemInst[i]->VAddr + MemInst[i]->Size - 1;
		Mem.Size = MemInst[i]->Size;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK_EQUAL(MemInst[i], Node);

	}

	for (i = 0; i < NUM; i++) {
		Mem.VAddr = MemInst[i]->VAddr;
		Mem.Size = MemInst[i]->Size;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK_EQUAL(MemInst[i], Node);
		RC = XAie_SimMemSyncForCPUVAddr(&DevInst, Mem.VAddr, Mem.Size);
		CHECK_EQUAL(RC, 0);

		Mem.VAddr = (char *)MemInst[i]->VAddr + 64;
		Mem.Size = MemInst[i]->Size;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK_EQUAL(MemInst[i], Node);
		RC = XAie_SimMemSyncForCPUVAddr(&DevInst, Mem.VAddr, Mem.Size);
		CHECK_EQUAL(RC, XAIE_INVALID_RANGE);

		Mem.VAddr = (char *)MemInst[i]->VAddr + 128;
		Mem.Size = 64;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK_EQUAL(MemInst[i], Node);
		RC = XAie_SimMemSyncForCPUVAddr(&DevInst, Mem.VAddr, Mem.Size);
		CHECK_EQUAL(RC, 0);

		Mem.VAddr = (char *)MemInst[i]->VAddr - 128;
		Mem.Size = 680;
		RC = XAie_SimMemSyncForCPUVAddr(&DevInst, Mem.VAddr, Mem.Size);
		CHECK(RC != 0);
	}

	for (i = 0; i < NUM; i++) {
		Mem.VAddr = MemInst[i]->VAddr;
		Mem.Size = MemInst[i]->Size;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK_EQUAL(MemInst[i], Node);
		RC = XAie_SimMemSyncForDevVAddr(&DevInst, Mem.VAddr, Mem.Size);
		CHECK_EQUAL(RC, 0);

		Mem.VAddr = (char *)MemInst[i]->VAddr + 64;
		Mem.Size = MemInst[i]->Size;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK_EQUAL(MemInst[i], Node);
		RC = XAie_SimMemSyncForDevVAddr(&DevInst, Mem.VAddr, Mem.Size);
		CHECK_EQUAL(RC, XAIE_INVALID_RANGE);

		Mem.VAddr = (char *)MemInst[i]->VAddr + 128;
		Mem.Size = 64;
		Node = (XAie_MemInst *)btree4_search(&IOInst->btree, &Mem);
		CHECK_EQUAL(MemInst[i], Node);
		RC = XAie_SimMemSyncForDevVAddr(&DevInst, Mem.VAddr, Mem.Size);
		CHECK_EQUAL(RC, 0);

		Mem.VAddr = (char *)MemInst[i]->VAddr - 128;
		Mem.Size = 680;
		RC = XAie_SimMemSyncForDevVAddr(&DevInst, Mem.VAddr, Mem.Size);
		CHECK(RC != 0);

	}

	for (i = 0; i < NUM; i++)
		XAie_MemFree(MemInst[i]);

#endif /*__AIESIM__*/
}
#endif  /*#ifndef FILES_MISSING*/
