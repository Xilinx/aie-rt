/******************************************************************************
* Copyright (C) 2022 AMD.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_util.h
* @{
*
* This header file defines lightweight utility helpers for the AIE lite
* driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  dsteger  7/10/2022  Initial creation
* </pre>
*
******************************************************************************/

#ifndef XAIE_LITE_UTIL_H
#define XAIE_LITE_UTIL_H

#include "xaiegbl_defs.h"
#include "xaie_lite_hwcfg.h"

#if defined(XAIE_FEATURE_LITE_UTIL)
/************************** Macro Definitions *****************************/

#define XAie_LGetColRangeBufSize(NumCols) { \
		sizeof(XAie_Col_Status) * NumCols \
	}

/************************** Variable Definitions *****************************/

/* Data structure to capture the dma status */
typedef struct {
	u32 S2MMStatus;
	u32 MM2SStatus;
} XAie_DmaStatus;

/* Data structure to capture the core tile status */
typedef struct {
	XAie_DmaStatus dma[XAIE_TILE_DMA_NUM_CH];
	u8  LockValue[XAIE_TILE_NUM_LOCKS];
	u32 CoreStatus;
	u32 ProgramCounter;
	u32 StackPtr;
	u32 LinkReg;
} XAie_Core_Tile_Status;

/* Data structure to capture the mem tile status */
#if (XAIE_DEV_SINGLE_GEN != XAIE_DEV_GEN_AIE)
typedef struct {
	XAie_DmaStatus dma[XAIE_MEM_TILE_DMA_NUM_CH];
	u8 LockValue[XAIE_MEM_TILE_NUM_LOCKS];
} XAie_Mem_Tile_Status;
#endif

/* Data structure to capture the shim tile status */
typedef struct {
	XAie_DmaStatus dma[XAIE_SHIM_DMA_NUM_CH];
	u8 LockValue[XAIE_SHIM_NUM_LOCKS];
} XAie_Shim_Tile_Status;

/* Data structure to capture column status */
typedef struct {
	XAie_Core_Tile_Status CoreTile[XAIE_AIE_TILE_NUM_ROWS];
#if (XAIE_DEV_SINGLE_GEN != XAIE_DEV_GEN_AIE)
	XAie_Mem_Tile_Status MemTile[XAIE_MEM_TILE_NUM_ROWS];
#endif
	XAie_Shim_Tile_Status ShimTile[XAIE_SHIM_NUM_ROWS];
} XAie_Col_Status;

/************************** Function Prototypes  *****************************/
#endif /* XAIE_FEATURE_LITE_UTIL */

#endif /* end of protection macro */

/** @} */
