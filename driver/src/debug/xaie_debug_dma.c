/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_debug_dma.c
* @{
*
* This file contains routines for DMA transaction count debug APIs.
* These APIs use performance counters to track DMA Start and End events,
* exposing how many DMA work units were started and how many BDs (i.e.,
* data deliveries) completed for verifying DMA progress and debugging hangs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   yosong  04/30/2026  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_debug_dma.h"
#include "xaie_events.h"
#include "xaie_helper.h"
#include "xaie_perfcnt.h"

/***************************** Macro Definitions *****************************/
#define XAIE_DMA_TXN_COUNTERS_PER_CH	2U

/***************************** Type Definitions ******************************/
typedef struct {
	XAie_ModuleType Module;
	XAie_Events StartEvent;
	XAie_Events EndEvent;
} XAie_DmaTxnChEvents;

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* Resolve the module type and Start / End events for a given tile type and
* DMA channel flag. The generation determines the Start event; the End event
* is always BD-level so EndCount directly reflects how many BDs (= data
* deliveries) the channel completed.
*   - Gen1 AIE:        Start = START_BD,   End = FINISHED_BD  (symmetric pair)
*   - AIE-ML and later: Start = START_TASK, End = FINISHED_BD  (asymmetric:
*                       one Start per task push, one End per BD completion)
*
* @param	TileType: Tile type from GetTTypefromLoc
* @param	DevGen: Device generation (XAIE_DEV_GEN_*)
* @param	DmaChFlag: Single-bit DMA channel flag (e.g. XAIE_DMA_CH_S2MM_0)
* @param	Events: Output structure populated on success
*
* @return	XAIE_OK on success, XAIE_INVALID_ARGS for unsupported combos.
*
******************************************************************************/
static AieRC _XAie_DmaTxnCountGetChEvents(u8 TileType, u8 DevGen,
		u32 DmaChFlag, XAie_DmaTxnChEvents *Events)
{
	u8 IsGen1 = (DevGen == XAIE_DEV_GEN_AIE) ? 1U : 0U;

	switch (TileType) {
	case XAIEGBL_TILE_TYPE_AIETILE:
		Events->Module = XAIE_MEM_MOD;
		switch (DmaChFlag) {
		case XAIE_DMA_CH_S2MM_0:
			Events->StartEvent = IsGen1 ?
				XAIE_EVENT_DMA_S2MM_0_START_BD_MEM :
				XAIE_EVENT_DMA_S2MM_0_START_TASK_MEM;
			Events->EndEvent = XAIE_EVENT_DMA_S2MM_0_FINISHED_BD_MEM;
			return XAIE_OK;
		case XAIE_DMA_CH_S2MM_1:
			Events->StartEvent = IsGen1 ?
				XAIE_EVENT_DMA_S2MM_1_START_BD_MEM :
				XAIE_EVENT_DMA_S2MM_1_START_TASK_MEM;
			Events->EndEvent = XAIE_EVENT_DMA_S2MM_1_FINISHED_BD_MEM;
			return XAIE_OK;
		case XAIE_DMA_CH_MM2S_0:
			Events->StartEvent = IsGen1 ?
				XAIE_EVENT_DMA_MM2S_0_START_BD_MEM :
				XAIE_EVENT_DMA_MM2S_0_START_TASK_MEM;
			Events->EndEvent = XAIE_EVENT_DMA_MM2S_0_FINISHED_BD_MEM;
			return XAIE_OK;
		case XAIE_DMA_CH_MM2S_1:
			Events->StartEvent = IsGen1 ?
				XAIE_EVENT_DMA_MM2S_1_START_BD_MEM :
				XAIE_EVENT_DMA_MM2S_1_START_TASK_MEM;
			Events->EndEvent = XAIE_EVENT_DMA_MM2S_1_FINISHED_BD_MEM;
			return XAIE_OK;
		default:
			return XAIE_INVALID_ARGS;
		}
	case XAIEGBL_TILE_TYPE_MEMTILE:
		/* Mem tile only exists on AIE-ML and later (Gen1 AIE has no
		 * mem tile), so no IsGen1 branch is needed. Channel flags map
		 * to SEL0/SEL1 event taps; caller routes a physical channel
		 * onto a tap with XAie_EventSelectDmaChannel(). */
		Events->Module = XAIE_MEM_MOD;
		switch (DmaChFlag) {
		case XAIE_DMA_CH_S2MM_0:
			Events->StartEvent =
				XAIE_EVENT_DMA_S2MM_SEL0_START_TASK_MEM_TILE;
			Events->EndEvent =
				XAIE_EVENT_DMA_S2MM_SEL0_FINISHED_BD_MEM_TILE;
			return XAIE_OK;
		case XAIE_DMA_CH_S2MM_1:
			Events->StartEvent =
				XAIE_EVENT_DMA_S2MM_SEL1_START_TASK_MEM_TILE;
			Events->EndEvent =
				XAIE_EVENT_DMA_S2MM_SEL1_FINISHED_BD_MEM_TILE;
			return XAIE_OK;
		case XAIE_DMA_CH_MM2S_0:
			Events->StartEvent =
				XAIE_EVENT_DMA_MM2S_SEL0_START_TASK_MEM_TILE;
			Events->EndEvent =
				XAIE_EVENT_DMA_MM2S_SEL0_FINISHED_BD_MEM_TILE;
			return XAIE_OK;
		case XAIE_DMA_CH_MM2S_1:
			Events->StartEvent =
				XAIE_EVENT_DMA_MM2S_SEL1_START_TASK_MEM_TILE;
			Events->EndEvent =
				XAIE_EVENT_DMA_MM2S_SEL1_FINISHED_BD_MEM_TILE;
			return XAIE_OK;
		default:
			return XAIE_INVALID_ARGS;
		}
	case XAIEGBL_TILE_TYPE_SHIMNOC:
	case XAIEGBL_TILE_TYPE_SHIMPL:
		Events->Module = XAIE_PL_MOD;
		switch (DmaChFlag) {
		case XAIE_DMA_CH_S2MM_0:
			Events->StartEvent = IsGen1 ?
				XAIE_EVENT_DMA_S2MM_0_START_BD_PL :
				XAIE_EVENT_DMA_S2MM_0_START_TASK_PL;
			Events->EndEvent = XAIE_EVENT_DMA_S2MM_0_FINISHED_BD_PL;
			return XAIE_OK;
		case XAIE_DMA_CH_S2MM_1:
			Events->StartEvent = IsGen1 ?
				XAIE_EVENT_DMA_S2MM_1_START_BD_PL :
				XAIE_EVENT_DMA_S2MM_1_START_TASK_PL;
			Events->EndEvent = XAIE_EVENT_DMA_S2MM_1_FINISHED_BD_PL;
			return XAIE_OK;
		case XAIE_DMA_CH_MM2S_0:
			Events->StartEvent = IsGen1 ?
				XAIE_EVENT_DMA_MM2S_0_START_BD_PL :
				XAIE_EVENT_DMA_MM2S_0_START_TASK_PL;
			Events->EndEvent = XAIE_EVENT_DMA_MM2S_0_FINISHED_BD_PL;
			return XAIE_OK;
		case XAIE_DMA_CH_MM2S_1:
			Events->StartEvent = IsGen1 ?
				XAIE_EVENT_DMA_MM2S_1_START_BD_PL :
				XAIE_EVENT_DMA_MM2S_1_START_TASK_PL;
			Events->EndEvent = XAIE_EVENT_DMA_MM2S_1_FINISHED_BD_PL;
			return XAIE_OK;
		default:
			return XAIE_INVALID_ARGS;
		}
	default:
		return XAIE_INVALID_TILE;
	}
}

/*****************************************************************************/
/**
* Count the number of channels (set bits) in a DMA channel bitmask, considering
* only the channel bits we recognize (XAIE_DMA_CH_ALL).
*
* @param	DmaChGroup: Bitmask of DMA channels
*
* @return	Number of channels selected.
*
******************************************************************************/
static u32 _XAie_DmaTxnCountPopCount(u32 DmaChGroup)
{
	u32 Count = 0U;
	u32 Bits = DmaChGroup & XAIE_DMA_CH_ALL;

	while (Bits != 0U) {
		Bits &= (Bits - 1U);
		Count++;
	}
	return Count;
}

/*****************************************************************************/
/**
* Get the maximum number of perf counters available in the module used for
* DMA event tracking on the given tile type.
*
* @param	DevInst: Device Instance
* @param	TileType: Tile type
* @param	Module: Module to query (XAIE_MEM_MOD or XAIE_PL_MOD)
*
* @return	Number of perf counters available, or 0 if module not present.
*
******************************************************************************/
static u8 _XAie_DmaTxnCountGetMaxCounters(XAie_DevInst *DevInst, u8 TileType,
		XAie_ModuleType Module)
{
	const XAie_PerfMod *PerfMod;

	if (Module == XAIE_PL_MOD) {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[0U];
	} else {
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[Module];
	}

	if (PerfMod == XAIE_NULL) {
		return 0U;
	}
	return PerfMod->MaxCounterVal;
}

/*****************************************************************************/
/**
* Configure a single tile's perf counters for the requested DMA channels.
* Each selected channel consumes a counter pair: one for the Start event and
* one for the End event. The exact event family depends on the device
* generation (BD-level on Gen1, task-level on AIE-ML+); see
* _XAie_DmaTxnCountGetChEvents. Counters are packed starting from index 0 in
* bit order so Disable/Get can use the same algorithm to locate them.
*
* @param	DevInst: Device Instance
* @param	Loc: Tile location
* @param	DmaChGroup: Bitmask of DMA channels
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal API. On partial failure, caller should invoke
*		XAie_DmaTxnCountDisable for cleanup.
*
******************************************************************************/
static AieRC _XAie_DmaTxnCountEnableTile(XAie_DevInst *DevInst,
		XAie_LocType Loc, u32 DmaChGroup)
{
	AieRC RC;
	u8 TileType, DevGen, MaxCounters, CounterIdx = 0U;
	XAie_ModuleType Module = XAIE_MEM_MOD;
	u32 NumChannels;
	u32 Bit;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if (TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile location\n");
		return XAIE_INVALID_TILE;
	}
	DevGen = DevInst->DevProp.DevGen;

	/*
	 * Module is determined by tile type: shim uses PL, AIE/Mem tiles use
	 * MEM. Resolve it once via the first set channel.
	 */
	for (Bit = 1U; Bit <= XAIE_DMA_CH_ALL; Bit <<= 1U) {
		if ((DmaChGroup & Bit) != 0U) {
			XAie_DmaTxnChEvents Probe;

			RC = _XAie_DmaTxnCountGetChEvents(TileType, DevGen,
					Bit, &Probe);
			if (RC != XAIE_OK) {
				return RC;
			}
			Module = Probe.Module;
			break;
		}
	}

	NumChannels = _XAie_DmaTxnCountPopCount(DmaChGroup);
	MaxCounters = _XAie_DmaTxnCountGetMaxCounters(DevInst, TileType,
			Module);
	if ((NumChannels * XAIE_DMA_TXN_COUNTERS_PER_CH) >
			(u32)MaxCounters) {
		XAIE_ERROR("Tile (%u,%u): %u channels need %u perf counters, "
			"only %u available\n",
			Loc.Col, Loc.Row, NumChannels,
			NumChannels * XAIE_DMA_TXN_COUNTERS_PER_CH,
			MaxCounters);
		return XAIE_ERR;
	}

	for (Bit = 1U; Bit <= XAIE_DMA_CH_ALL; Bit <<= 1U) {
		XAie_DmaTxnChEvents ChEvents;
		u8 StartCounter, EndCounter;

		if ((DmaChGroup & Bit) == 0U) {
			continue;
		}

		RC = _XAie_DmaTxnCountGetChEvents(TileType, DevGen, Bit,
				&ChEvents);
		if (RC != XAIE_OK) {
			XAIE_ERROR("Tile (%u,%u): no events for channel "
				"flag 0x%x\n", Loc.Col, Loc.Row, Bit);
			return RC;
		}

		StartCounter = CounterIdx;
		EndCounter = (u8)(CounterIdx + 1U);
		CounterIdx = (u8)(CounterIdx + XAIE_DMA_TXN_COUNTERS_PER_CH);

		/*
		 * Per AIE arch spec, setting the same event for both Start
		 * and Stop makes the counter increment once per occurrence.
		 */
		RC = XAie_PerfCounterControlSet(DevInst, Loc, ChEvents.Module,
				StartCounter, ChEvents.StartEvent,
				ChEvents.StartEvent);
		if (RC != XAIE_OK) {
			XAIE_ERROR("Tile (%u,%u): failed to configure start "
				"counter %u\n", Loc.Col, Loc.Row,
				StartCounter);
			return RC;
		}

		RC = XAie_PerfCounterControlSet(DevInst, Loc, ChEvents.Module,
				EndCounter, ChEvents.EndEvent,
				ChEvents.EndEvent);
		if (RC != XAIE_OK) {
			XAIE_ERROR("Tile (%u,%u): failed to configure end "
				"counter %u\n", Loc.Col, Loc.Row, EndCounter);
			return RC;
		}

		/* Zero the counters so reads start from a known baseline. */
		(void)XAie_PerfCounterReset(DevInst, Loc, ChEvents.Module,
				StartCounter);
		(void)XAie_PerfCounterReset(DevInst, Loc, ChEvents.Module,
				EndCounter);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API enables DMA transaction counting on a group of tiles by configuring
* performance counters to count Start/End DMA events. Each selected channel
* consumes 2 perf counters per tile (one for the Start event, one for the End
* event).
*
* The End counter always counts BD completions (= data deliveries) on every
* generation. The Start counter counts whatever Start event the chip exposes:
*   - Gen1 AIE (XAIE_DEV_GEN_AIE): Start = START_BD, End = FINISHED_BD.
*     Symmetric pair: one Start tick and one End tick per BD entered/done.
*   - AIE-ML and later: Start = START_TASK, End = FINISHED_BD.
*     Asymmetric pair: one Start tick per task pushed, one End tick per BD
*     completed within the task. For example, a single task that walks a
*     3-BD chain reports StartCount=1, EndCount=3.
*
* Implications for callers:
*   - EndCount directly answers "how many BDs of data were transferred?"
*     on every chip. This is the primary debugging signal.
*   - Hang detection: take two snapshots over time; if EndCount stops
*     advancing while data is supposed to flow, the channel is hung.
*   - Completion test: on Gen1, all-done means EndCount == StartCount;
*     on AIE-ML+ the ratio depends on BDs-per-task and repeatCount, so
*     callers compare EndCount against an expected total they know.
*
* Counter budget per tile (rejected with XAIE_ERR if exceeded):
*   - AIE tile mem module: 2 counters (1 channel)
*   - Mem tile (AIE-ML+):  4 counters (2 channels)
*   - Shim tile:           2 counters (1 channel); 6 on AIE2PS (3 channels)
*
* Mem-tile note: flags map to SEL0/SEL1 event taps. Caller selects which
* physical channel each tap watches via XAie_EventSelectDmaChannel().
*
* @param	DevInst: Device Instance
* @param	SubParts: Array of rectangular tile regions to monitor
* @param	NumSubParts: Number of entries in the SubParts array
* @param	DmaChGroup: Bitmask of DMA channels to monitor
*			(e.g. XAIE_DMA_CH_S2MM_0 | XAIE_DMA_CH_MM2S_0)
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		On failure midway through a multi-tile call, callers should
*		invoke XAie_DmaTxnCountDisable to clean up partial state.
*
******************************************************************************/
AieRC XAie_DmaTxnCountEnable(XAie_DevInst *DevInst, XAie_SubPartition *SubParts,
		u32 NumSubParts, u32 DmaChGroup)
{
	AieRC RC;
	u32 i;

	if ((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	if ((SubParts == XAIE_NULL) || (NumSubParts == 0U)) {
		XAIE_ERROR("Invalid sub-partition array\n");
		return XAIE_INVALID_ARGS;
	}

	if ((DmaChGroup & XAIE_DMA_CH_ALL) == 0U) {
		XAIE_ERROR("Empty DMA channel group\n");
		return XAIE_INVALID_ARGS;
	}

	for (i = 0U; i < NumSubParts; i++) {
		const XAie_Range *Cols = &SubParts[i].ColRange;
		const XAie_Range *Rows = &SubParts[i].RowRange;
		u8 Col, Row;

		if ((Cols->Num == 0U) || (Rows->Num == 0U)) {
			XAIE_ERROR("Sub-partition %u has empty range\n", i);
			return XAIE_INVALID_ARGS;
		}

		for (Col = Cols->Start;
				Col < (u8)(Cols->Start + Cols->Num); Col++) {
			for (Row = Rows->Start;
					Row < (u8)(Rows->Start + Rows->Num);
					Row++) {
				RC = _XAie_DmaTxnCountEnableTile(DevInst,
						XAie_TileLoc(Col, Row),
						DmaChGroup);
				if (RC != XAIE_OK) {
					return RC;
				}
			}
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* Tear down the perf counters configured by _XAie_DmaTxnCountEnableTile for
* this tile. Walks DmaChGroup in the same bit order as Enable so the counter
* indices line up. Best-effort: continues on error and reports the first
* failure, so a partially-failed Enable can still be fully cleaned up.
*
* @param	DevInst: Device Instance
* @param	Loc: Tile location
* @param	DmaChGroup: Bitmask of DMA channels to disable
*
* @return	XAIE_OK on success, first error code seen otherwise.
*
* @note		Internal API.
*
******************************************************************************/
static AieRC _XAie_DmaTxnCountDisableTile(XAie_DevInst *DevInst,
		XAie_LocType Loc, u32 DmaChGroup)
{
	AieRC RC;
	AieRC FirstErr = XAIE_OK;
	u8 TileType, DevGen, MaxCounters, CounterIdx = 0U;
	XAie_ModuleType Module = XAIE_MEM_MOD;
	u32 NumChannels;
	u32 Bit;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if (TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile location\n");
		return XAIE_INVALID_TILE;
	}
	DevGen = DevInst->DevProp.DevGen;

	for (Bit = 1U; Bit <= XAIE_DMA_CH_ALL; Bit <<= 1U) {
		if ((DmaChGroup & Bit) != 0U) {
			XAie_DmaTxnChEvents Probe;

			RC = _XAie_DmaTxnCountGetChEvents(TileType, DevGen,
					Bit, &Probe);
			if (RC != XAIE_OK) {
				return RC;
			}
			Module = Probe.Module;
			break;
		}
	}

	NumChannels = _XAie_DmaTxnCountPopCount(DmaChGroup);
	MaxCounters = _XAie_DmaTxnCountGetMaxCounters(DevInst, TileType,
			Module);
	if ((NumChannels * XAIE_DMA_TXN_COUNTERS_PER_CH) >
			(u32)MaxCounters) {
		XAIE_ERROR("Tile (%u,%u): %u channels exceed counter "
			"budget %u (DmaChGroup mismatch with Enable?)\n",
			Loc.Col, Loc.Row, NumChannels, MaxCounters);
		return XAIE_ERR;
	}

	for (Bit = 1U; Bit <= XAIE_DMA_CH_ALL; Bit <<= 1U) {
		XAie_DmaTxnChEvents ChEvents;
		u8 StartCounter, EndCounter;

		if ((DmaChGroup & Bit) == 0U) {
			continue;
		}

		RC = _XAie_DmaTxnCountGetChEvents(TileType, DevGen, Bit,
				&ChEvents);
		if (RC != XAIE_OK) {
			if (FirstErr == XAIE_OK) {
				FirstErr = RC;
			}
			continue;
		}

		StartCounter = CounterIdx;
		EndCounter = (u8)(CounterIdx + 1U);
		CounterIdx = (u8)(CounterIdx + XAIE_DMA_TXN_COUNTERS_PER_CH);

		RC = XAie_PerfCounterControlReset(DevInst, Loc,
				ChEvents.Module, StartCounter);
		if ((RC != XAIE_OK) && (FirstErr == XAIE_OK)) {
			FirstErr = RC;
		}

		RC = XAie_PerfCounterControlReset(DevInst, Loc,
				ChEvents.Module, EndCounter);
		if ((RC != XAIE_OK) && (FirstErr == XAIE_OK)) {
			FirstErr = RC;
		}

		(void)XAie_PerfCounterReset(DevInst, Loc, ChEvents.Module,
				StartCounter);
		(void)XAie_PerfCounterReset(DevInst, Loc, ChEvents.Module,
				EndCounter);
	}

	return FirstErr;
}

/*****************************************************************************/
/**
* This API disables DMA transaction counting on a group of tiles by tearing
* down the performance counters that XAie_DmaTxnCountEnable configured. Both
* the Start and End counters of every selected channel are reset: the control
* register is cleared (un-binds the events) and the counter value is zeroed.
*
* The DmaChGroup must match the value passed to XAie_DmaTxnCountEnable so the
* same counter indices are torn down. Disable is best-effort across the tile
* group: on a per-tile failure it still attempts the remaining tiles and
* counters, then returns the first error seen (XAIE_OK if all succeeded).
*
* @param	DevInst: Device Instance
* @param	SubParts: Array of rectangular tile regions to disable
* @param	NumSubParts: Number of entries in the SubParts array
* @param	DmaChGroup: Bitmask of DMA channels to disable
*			(must match the value passed to Enable)
*
* @return	XAIE_OK on success, first error code seen otherwise.
*
******************************************************************************/
AieRC XAie_DmaTxnCountDisable(XAie_DevInst *DevInst,
		XAie_SubPartition *SubParts, u32 NumSubParts, u32 DmaChGroup)
{
	AieRC RC;
	AieRC FirstErr = XAIE_OK;
	u32 i;

	if ((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	if ((SubParts == XAIE_NULL) || (NumSubParts == 0U)) {
		XAIE_ERROR("Invalid sub-partition array\n");
		return XAIE_INVALID_ARGS;
	}

	if ((DmaChGroup & XAIE_DMA_CH_ALL) == 0U) {
		XAIE_ERROR("Empty DMA channel group\n");
		return XAIE_INVALID_ARGS;
	}

	for (i = 0U; i < NumSubParts; i++) {
		const XAie_Range *Cols = &SubParts[i].ColRange;
		const XAie_Range *Rows = &SubParts[i].RowRange;
		u8 Col, Row;

		if ((Cols->Num == 0U) || (Rows->Num == 0U)) {
			if (FirstErr == XAIE_OK) {
				FirstErr = XAIE_INVALID_ARGS;
			}
			continue;
		}

		for (Col = Cols->Start;
				Col < (u8)(Cols->Start + Cols->Num); Col++) {
			for (Row = Rows->Start;
					Row < (u8)(Rows->Start + Rows->Num);
					Row++) {
				RC = _XAie_DmaTxnCountDisableTile(DevInst,
						XAie_TileLoc(Col, Row),
						DmaChGroup);
				if ((RC != XAIE_OK) &&
						(FirstErr == XAIE_OK)) {
					FirstErr = RC;
				}
			}
		}
	}

	return FirstErr;
}

/*****************************************************************************/
/**
* Compute the counter pair indices that Enable assigned to DmaChFlag, given
* the original DmaChGroup. Enable packs counter pairs in low-to-high bit
* order: the Nth set bit consumes counter indices 2*N (Start) and 2*N+1
* (End). DmaChFlag must be one of the bits set in DmaChGroup.
*
* @param	DmaChGroup: Bitmask passed to Enable
* @param	DmaChFlag: Single channel flag to locate within DmaChGroup
* @param	StartCounter: Output Start counter index
* @param	EndCounter: Output End counter index
*
* @return	XAIE_OK on success, XAIE_INVALID_ARGS if DmaChFlag is not in
*		DmaChGroup.
*
******************************************************************************/
static AieRC _XAie_DmaTxnCountResolveCounter(u32 DmaChGroup, u32 DmaChFlag,
		u8 *StartCounter, u8 *EndCounter)
{
	u32 Bit;
	u32 Slot = 0U;

	for (Bit = 1U; Bit <= XAIE_DMA_CH_ALL; Bit <<= 1U) {
		if ((DmaChGroup & Bit) == 0U) {
			continue;
		}
		if (Bit == DmaChFlag) {
			*StartCounter = (u8)(Slot *
					XAIE_DMA_TXN_COUNTERS_PER_CH);
			*EndCounter = (u8)(*StartCounter + 1U);
			return XAIE_OK;
		}
		Slot++;
	}

	return XAIE_INVALID_ARGS;
}

/*****************************************************************************/
/**
* This API reads the DMA transaction counts for one channel previously set up
* by XAie_DmaTxnCountEnable. See XAie_DmaTxnCountEnable for the per-generation
* event mapping. EndCount always reflects BD completions (= data deliveries);
* StartCount reflects BD pushes on Gen1 and task pushes on AIE-ML+.
*
* The caller passes the same DmaChGroup that was given to Enable, plus the
* single DmaChFlag bit identifying which channel to read. Counter indices are
* derived deterministically from bit position in DmaChGroup (matching the
* repo's FAL-style "track-the-counter-index" convention rather than reading
* the perf-control register back).
*
* @param	DevInst: Device Instance
* @param	Loc: Tile location
* @param	DmaChGroup: Bitmask passed to XAie_DmaTxnCountEnable on this
*			tile
* @param	DmaChFlag: Single channel flag (e.g. XAIE_DMA_CH_S2MM_0); must
*			be one of the bits set in DmaChGroup
* @param	StartCount: Pointer to store Start event count
* @param	EndCount: Pointer to store End event count (BD completions)
*
* @return	XAIE_OK on success, error code on failure.
*
******************************************************************************/
AieRC XAie_DmaTxnCountGet(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 DmaChGroup, u32 DmaChFlag, u32 *StartCount,
		u32 *EndCount)
{
	AieRC RC;
	u8 TileType;
	u8 StartCounter = 0U;
	u8 EndCounter = 0U;
	XAie_ModuleType Module;

	if ((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	if ((StartCount == XAIE_NULL) || (EndCount == XAIE_NULL)) {
		XAIE_ERROR("Null count pointer\n");
		return XAIE_INVALID_ARGS;
	}

	if ((DmaChGroup & XAIE_DMA_CH_ALL) == 0U) {
		XAIE_ERROR("Empty DMA channel group\n");
		return XAIE_INVALID_ARGS;
	}

	if (((DmaChFlag & XAIE_DMA_CH_ALL) == 0U) ||
			(_XAie_DmaTxnCountPopCount(DmaChFlag) != 1U)) {
		XAIE_ERROR("DmaChFlag must be a single channel flag\n");
		return XAIE_INVALID_ARGS;
	}

	if ((DmaChFlag & DmaChGroup) != DmaChFlag) {
		XAIE_ERROR("DmaChFlag 0x%x not present in DmaChGroup 0x%x\n",
			DmaChFlag, DmaChGroup);
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	switch (TileType) {
	case XAIEGBL_TILE_TYPE_AIETILE:
	case XAIEGBL_TILE_TYPE_MEMTILE:
		Module = XAIE_MEM_MOD;
		break;
	case XAIEGBL_TILE_TYPE_SHIMNOC:
	case XAIEGBL_TILE_TYPE_SHIMPL:
		Module = XAIE_PL_MOD;
		break;
	default:
		XAIE_ERROR("Invalid tile location\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_DmaTxnCountResolveCounter(DmaChGroup, DmaChFlag,
			&StartCounter, &EndCounter);
	if (RC != XAIE_OK) {
		return RC;
	}

	RC = XAie_PerfCounterGet(DevInst, Loc, Module, StartCounter,
			StartCount);
	if (RC != XAIE_OK) {
		return RC;
	}

	return XAie_PerfCounterGet(DevInst, Loc, Module, EndCounter,
			EndCount);
}
