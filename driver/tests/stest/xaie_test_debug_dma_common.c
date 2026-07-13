/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xaiengine.h>

#include "xaie_test_debug_dma_common.h"

/************************** Constant Definitions *****************************/
#define DATA_MEM_INPUT_ADDR	0x4000U
#define DATA_MEM_OUTPUT_ADDR	0x3000U
#define NUM_ELEMS		32U

#define LOCK_FOR_WRITE		0
#define LOCK_FOR_READ		1

/* Bound the pending-BD wait so a wedged DMA doesn't hang the harness. */
#define PENDING_BD_RETRY_LIMIT	10000U

/************************** Function Definitions *****************************/
int _XAie_TestDebugDmaLoopback(XAie_DevInst *DevInst,
		XAie_LocType *MonTiles, u32 ChGroup)
{
	AieRC RC;
	XAie_LocType Tile_1 = MonTiles[0];
	XAie_LocType Tile_2 = MonTiles[1];
	XAie_DmaDesc Tile_1_MM2S;
	XAie_DmaDesc Tile_1_S2MM;
	XAie_DmaDesc Tile_2_MM2S;
	XAie_DmaDesc Tile_2_S2MM;
	uint32_t data[NUM_ELEMS];
	u8 PendingBd = 1U;
	u32 EndCount = 0U;

	for (u8 i = 0U; i < NUM_ELEMS; i++) {
		data[i] = (uint32_t)(rand() % 127);
	}

	RC = XAie_DataMemBlockWrite(DevInst, Tile_1, DATA_MEM_INPUT_ADDR,
			(void *)data, sizeof(uint32_t) * NUM_ELEMS);
	if (RC != XAIE_OK) {
		printf("[stest/test_debug_dma] "
			"DataMemBlockWrite failed: 0x%x\n", RC);
		return -1;
	}

	RC |= XAie_StrmConnCctEnable(DevInst, Tile_1, DMA, 0, NORTH, 0);
	RC |= XAie_StrmConnCctEnable(DevInst, Tile_2, SOUTH, 0, DMA, 0);
	RC |= XAie_StrmConnCctEnable(DevInst, Tile_2, DMA, 0, SOUTH, 0);
	RC |= XAie_StrmConnCctEnable(DevInst, Tile_1, NORTH, 0, DMA, 0);
	if (RC != XAIE_OK) {
		printf("[stest/test_debug_dma] "
			"Stream switch setup failed: 0x%x\n", RC);
		return -1;
	}

	RC |= XAie_DmaDescInit(DevInst, &Tile_1_MM2S, Tile_1);
	RC |= XAie_DmaDescInit(DevInst, &Tile_1_S2MM, Tile_1);
	RC |= XAie_DmaDescInit(DevInst, &Tile_2_MM2S, Tile_2);
	RC |= XAie_DmaDescInit(DevInst, &Tile_2_S2MM, Tile_2);

	RC |= XAie_DmaSetAddrLen(&Tile_1_MM2S, DATA_MEM_INPUT_ADDR,
			NUM_ELEMS * sizeof(uint32_t));
	RC |= XAie_DmaSetAddrLen(&Tile_1_S2MM, DATA_MEM_OUTPUT_ADDR,
			NUM_ELEMS * sizeof(uint32_t));
	RC |= XAie_DmaSetAddrLen(&Tile_2_MM2S, DATA_MEM_INPUT_ADDR,
			NUM_ELEMS * sizeof(uint32_t));
	RC |= XAie_DmaSetAddrLen(&Tile_2_S2MM, DATA_MEM_INPUT_ADDR,
			NUM_ELEMS * sizeof(uint32_t));

	RC |= XAie_DmaSetLock(&Tile_2_S2MM, XAie_LockInit(5U, LOCK_FOR_WRITE),
			XAie_LockInit(5U, LOCK_FOR_READ));
	RC |= XAie_DmaSetLock(&Tile_2_MM2S, XAie_LockInit(5U, LOCK_FOR_READ),
			XAie_LockInit(5U, LOCK_FOR_WRITE));
	RC |= XAie_DmaSetLock(&Tile_1_MM2S, XAie_LockInit(6U, LOCK_FOR_WRITE),
			XAie_LockInit(6U, LOCK_FOR_READ));
	RC |= XAie_DmaSetLock(&Tile_1_S2MM, XAie_LockInit(6U, LOCK_FOR_READ),
			XAie_LockInit(6U, LOCK_FOR_WRITE));

	RC |= XAie_DmaEnableBd(&Tile_1_MM2S);
	RC |= XAie_DmaEnableBd(&Tile_1_S2MM);
	RC |= XAie_DmaEnableBd(&Tile_2_MM2S);
	RC |= XAie_DmaEnableBd(&Tile_2_S2MM);

	RC |= XAie_DmaWriteBd(DevInst, &Tile_1_MM2S, Tile_1, 1U);
	RC |= XAie_DmaWriteBd(DevInst, &Tile_1_S2MM, Tile_1, 9U);
	RC |= XAie_DmaWriteBd(DevInst, &Tile_2_MM2S, Tile_2, 1U);
	RC |= XAie_DmaWriteBd(DevInst, &Tile_2_S2MM, Tile_2, 9U);

	RC |= XAie_DmaChannelPushBdToQueue(DevInst, Tile_1, 0U, DMA_MM2S, 1U);
	RC |= XAie_DmaChannelPushBdToQueue(DevInst, Tile_1, 0U, DMA_S2MM, 9U);
	RC |= XAie_DmaChannelPushBdToQueue(DevInst, Tile_2, 0U, DMA_MM2S, 1U);
	RC |= XAie_DmaChannelPushBdToQueue(DevInst, Tile_2, 0U, DMA_S2MM, 9U);
	RC |= XAie_DmaChannelEnable(DevInst, Tile_1, 0U, DMA_MM2S);
	RC |= XAie_DmaChannelEnable(DevInst, Tile_1, 0U, DMA_S2MM);
	RC |= XAie_DmaChannelEnable(DevInst, Tile_2, 0U, DMA_MM2S);
	RC |= XAie_DmaChannelEnable(DevInst, Tile_2, 0U, DMA_S2MM);
	if (RC != XAIE_OK) {
		printf("[stest/test_debug_dma] "
			"DMA setup/start failed: 0x%x\n", RC);
		return -1;
	}

	for (u32 retry = 0U; PendingBd != 0U && retry < PENDING_BD_RETRY_LIMIT;
			retry++) {
		(void)XAie_DmaGetPendingBdCount(DevInst, Tile_1, 0U, DMA_S2MM,
				&PendingBd);
	}
	if (PendingBd != 0U) {
		printf("[stest/test_debug_dma] "
			"DMA never drained (PendingBd=%u); proceeding anyway "
			"to read counters.\n", PendingBd);
	}
	/* Give the perf counters time to latch on the simulator. */
	(void)sleep(1);

	for (u8 i = 0U; i < 2U; i++) {
		RC = XAie_DmaTxnCountGet(DevInst, MonTiles[i], ChGroup,
				XAIE_DMA_CH_MM2S_0, &EndCount);
		if (RC != XAIE_OK) {
			printf("[stest/test_debug_dma] "
				"XAie_DmaTxnCountGet failed on tile (%u,%u): "
				"0x%x\n", MonTiles[i].Col, MonTiles[i].Row,
				RC);
			return -1;
		}
		printf("[stest/test_debug_dma] tile (%u,%u) MM2S Ch 0: "
			"EndCount=%u\n",
			MonTiles[i].Col, MonTiles[i].Row, EndCount);
		if (EndCount == 0U) {
			printf("[stest/test_debug_dma] "
				"FAIL: EndCount stayed at 0 on (%u,%u)\n",
				MonTiles[i].Col, MonTiles[i].Row);
			return -1;
		}
	}

	return 0;
}
