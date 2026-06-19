/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xaie_test_debug_dma_common.h
* @{
*
* Shared helper for DMA transaction count debug system tests. The two test
* entry points (test_debug_dma_txn_count and test_debug_dma_txn_count_v2)
* differ only in how the perf counters are armed and released; the DMA
* loopback that exercises them is identical and lives in the helper.
*
******************************************************************************/
#ifndef XAIE_TEST_DEBUG_DMA_COMMON_H
#define XAIE_TEST_DEBUG_DMA_COMMON_H

#include <xaiengine.h>

/*
 * Tile coordinates used by both test variants. Column 1, row 2 and 3 are
 * AIE tiles on every supported AIE generation, which lets the same coords
 * work on the AIE-ML simulator and on Gen1/AIE2P/AIE2PS sims if/when those
 * become available.
 */
#define XAIE_TEST_DEBUG_DMA_COL		1U
#define XAIE_TEST_DEBUG_DMA_ROW_BASE	2U

/*****************************************************************************/
/**
* Run the standard two-tile MM2S<->S2MM DMA loopback used to exercise debug
* DMA transaction counters. The caller is responsible for arming the
* counters before this call (via XAie_DmaTxnCountEnable or
* XAie_PartitionInitialize_v2 with DmaTxnCountChGroup set) and for releasing
* them afterwards.
*
* On success the function reads the counters back via XAie_DmaTxnCountGet
* and returns 0; on failure it logs a diagnostic and returns -1.
*
* @param	DevInst: Device Instance
* @param	MonTiles: Two AIE tiles, MonTiles[1] one row north of [0].
* @param	ChGroup: DMA channel bitmask the counters are armed on.
*			 Currently only XAIE_DMA_CH_MM2S_0 is exercised.
*
* @return	0 on success, -1 on failure.
*
******************************************************************************/
int _XAie_TestDebugDmaLoopback(XAie_DevInst *DevInst,
		XAie_LocType *MonTiles, u32 ChGroup);

#endif /* XAIE_TEST_DEBUG_DMA_COMMON_H */
/** @} */
