/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_debug_dma.h
* @{
*
* Header file for DMA transaction count debug APIs.
*
* These APIs program a performance counter per channel to count how many BDs
* (i.e., data deliveries) a DMA channel has completed. This directly answers
* "how many BDs of data were transferred on this channel?" and, by sampling
* the count over time, "has this channel stalled?".
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date        Changes
* ----- ------   --------    --------------------------------------------------
* 1.0   yosong   04/30/2026  Initial creation
* 1.1   yosong   07/13/2026  Drop Start-event tracking; count BD completions
*                            (FINISHED_BD) only, per generation-agnostic
*                            single-counter design
*
* </pre>
*
******************************************************************************/
#ifndef XAIE_DEBUG_DMA_H
#define XAIE_DEBUG_DMA_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"

/**************************** Macro Definitions ******************************/
/*
 * DMA channel flags. Combine with bitwise OR to form a DmaChGroup.
 *
 * On AIE tile and Shim tile, flags map directly to physical channels 0 and 1.
 * On Mem tile, flags map to the SEL0/SEL1 event taps; caller programs which
 * physical channel each tap watches via XAie_EventSelectDmaChannel().
 */
#define XAIE_DMA_CH_S2MM_0	(1U << 0)
#define XAIE_DMA_CH_S2MM_1	(1U << 1)
#define XAIE_DMA_CH_MM2S_0	(1U << 2)
#define XAIE_DMA_CH_MM2S_1	(1U << 3)
#define XAIE_DMA_CH_ALL		(0xFU)

/************************** Function Prototypes  *****************************/
AieRC XAie_DmaTxnCountEnable(XAie_DevInst *DevInst,
		XAie_SubPartition *SubParts, u32 NumSubParts, u32 DmaChGroup);
AieRC XAie_DmaTxnCountDisable(XAie_DevInst *DevInst,
		XAie_SubPartition *SubParts, u32 NumSubParts, u32 DmaChGroup);
AieRC XAie_DmaTxnCountGet(XAie_DevInst *DevInst,
		XAie_LocType Loc, u32 DmaChGroup, u32 DmaChFlag,
		u32 *EndCount);

#endif		/* end of protection macro */
/** @} */
