// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// Copyright (c) 2022 Advanced Micro Devices, Inc. All Rights Reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace std;
using namespace xaiefal;

extern "C" {
extern const XAie_Backend DebugBackend;
}


AieRC Test_XAie_DebugIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	u32 *ioread = (u32 *)((char *)IOInst + RegOff);
	XAIE_ERROR("MP: %p, Mask: 0x%x, Value: 0x%x, TimeOutUs: 0x%d\n", (char *)ioread, Mask, Value, TimeOutUs);
	XAIE_ERROR("ioread: %u\n", *ioread);
	if ((*ioread & Mask) == Value)
		return XAIE_OK;
	return XAIE_ERR;
}

TEST_GROUP(DMA)
{
};

TEST(DMA, DmaWaitForDone) {
#define XAIEML_DMA_STATUS_CHNUM_OFFSET                  0x4U
	AieRC RC;
	XAie_Backend Backend;
	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
		XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
		XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
		XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
		XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);
	XAie_InstDeclare(DevInst, &ConfigPtr);
	XAie_LocType loc;
	uint8_t ChNum = 0;
	uint32_t ioread = 0;
	uint64_t addr;
	const XAie_DmaMod *DmaMod;
	u8 TileType;

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	loc.Row = 0;
	loc.Col = 0;

	RC = XAie_DmaWaitForDone(NULL, loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_INVALID_TILE, RC);

	loc.Row = 1;
	loc.Col = 3;
	ChNum = 0xff;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_INVALID_CHANNEL_NUM, RC);

	ChNum = 0;
	Backend = DebugBackend;
	Backend.Ops.MaskPoll = Test_XAie_DebugIO_MaskPoll;
	DevInst.Backend = &Backend;
	TileType = DevInst.DevOps->GetTTypefromLoc(&DevInst, loc);
	DmaMod = DevInst.DevProp.DevMod[TileType].DmaMod;
	addr = _XAie_GetTileAddr(&DevInst, loc.Row, loc.Col) + DmaMod->ChStatusBase +
	       DMA_S2MM * DmaMod->ChStatusOffset;
#if (AIE_GEN == 2)
	addr += ChNum * XAIEML_DMA_STATUS_CHNUM_OFFSET;
#endif
	/* Call to XAie_DmaWaitForDone() does an ioread from DevInst->IOInst
	 * as a base address, at offset = addr calculated above.
	 * Set the DevInst->IOInst to right offset so that XAie_DmaWaitForDone()
	 * would read the variable ioread.
	 */
	DevInst.IOInst = (char *)&ioread - addr;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_OK, RC);

	ioread = 0xffffffff;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);

	/* Status */
#if (AIE_GEN == 1)
	ioread = DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Mask;
#elif (AIE_GEN == 2)
	ioread = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.Status.Mask;
#endif
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);

#if (AIE_GEN == 1)
	/* StartQSize */
	ioread = DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Mask;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);

	/* StartQSize */
	ioread = DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.StartQSize.Mask;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);

	/* Stalled */
	ioread = DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Stalled.Mask;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);
#endif

#if (AIE_GEN == 2)
	/* TaskQSize */
	ioread = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.TaskQSize.Mask;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);

	/* ChannelRunning */
	ioread = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.ChannelRunning.Mask;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);

	/* StalledLockAcq */
	ioread = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockAcq.Mask;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);

	/* StalledLockRel */
	ioread = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledLockRel.Mask;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);

	/* StalledStreamStarve */
	ioread = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledStreamStarve.Mask;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);

	/* StalledTCT */
	ioread = DmaMod->ChProp->DmaChStatus->AieMlDmaChStatus.StalledTCT.Mask;
	RC = XAie_DmaWaitForDone(&(DevInst), loc, ChNum, DMA_S2MM, 0);
	CHECK_EQUAL(XAIE_ERR, RC);
#endif

}
