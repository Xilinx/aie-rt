/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaie_perfcnt.c
* @{
*
* This file contains routines for AIE performance counters
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 11/22/2019  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_perfcnt.h"
#include "xaie_events.h"

/*****************************************************************************/
/***************************** Macro Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/* This API reads the given counter for the given range of tiles
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @param        Counter:Performance Counter
* @return       XAIE_OK on success
*
* @note
*
******************************************************************************/
u32 XAie_PerfCounterGet(XAie_DevInst *DevInst, XAie_PerfCounters Counter,
							XAie_LocType Loc)
{
	AieRC RC;
	u32 CounterRegOffset;
	u64 CounterRegAddr;
	u8 TileType;
	const XAie_PerfMod *PerfMod;
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	PerfMod = DevInst->DevProp.DevMod[TileType].PerfMod;
	/*
	 * Checking if the counter is memory module counter of AIE tile
	 * XAie_PerfCounters Enum number corresponds to mem module - counter 4
	 * or 5, it is implied to be counter 0 or 1 of mem module of aie tile.
	 */
	if(Counter >= XAIE_MEMPERFCOUNTER_0){
		Counter -= XAIE_MEMPERFCOUNTER_0;
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[1];
	}

	/* Checking for valid Counter */
	if(Counter >= PerfMod->MaxCounterVal){
		XAieLib_print("Error: Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Get offset address based on Counter */
	CounterRegOffset = PerfMod->PerfCounterBaseAddr +
				((Counter)*PerfMod->PerfCounterOffsetAdd);

	u8 R = Loc.Row;
	u8 C = Loc.Col;
	/* Compute absolute address and write to register */
	CounterRegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, R ,C) + CounterRegOffset;

	return XAieGbl_Read32(CounterRegAddr);
}
/*****************************************************************************/
/* This API configures the control registers corresponding to the counters
*  with the start and stop event for the given tile
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the tile
* @param        Counter:Performance Counter
* @param        StartEvent:Event that triggers start to the counter
* @Param        StopEvent: Event that triggers stop to the counter
* @return       XAIE_OK on success
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterControlSet(XAie_DevInst *DevInst,XAie_LocType Loc,
	XAie_PerfCounters Counter, XAie_Events StartEvent, XAie_Events StopEvent)
{
	AieRC RC;
	u32 RegOffset, FldVal, FldMask;
	u64 RegAddr;
	u8 TileType, IntStartEvent, IntStopEvent;
	const XAie_PerfMod *PerfMod;
	const XAie_EvntMod *EvntMod;

	XAie_LocRange Range = { Loc, Loc, { 1U, 1U } };

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	PerfMod = DevInst->DevProp.DevMod[TileType].PerfMod;
	EvntMod = DevInst->DevProp.DevMod[TileType].EvntMod;
	/*
	 * Checking if the counter is memory module counter of AIE tile
	 * XAie_PerfCounters Enum number corresponds to mem module - counter 4
	 * or 5, it is implied to be counter 0 or 1 of mem module of aie tile.
	 */
	if(Counter >= XAIE_MEMPERFCOUNTER_0){
		Counter -= XAIE_MEMPERFCOUNTER_0;
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[1];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[1];
	}

	/* check if the event passed as input is corresponding to the module */
	if(StartEvent < EvntMod->EventMin || StartEvent > EvntMod->EventMax ||
		StopEvent < EvntMod->EventMin || StopEvent > EvntMod->EventMax){
		XAieLib_print("Error: Invalid Event id\n");
		return XAIE_INVALID_ARGS;
	}

	/* Subtract the module offset from event number */
	StartEvent -= EvntMod->EventMin;
	StopEvent -= EvntMod->EventMin;

	/* Getting the true event number from the enum to array mapping */
	IntStartEvent = EvntMod->XAie_EventNumber[StartEvent];
	IntStopEvent = EvntMod->XAie_EventNumber[StopEvent];

	/*checking for valid true event number */
	if(IntStartEvent == XAIE_EVENT_INVALID ||
			IntStopEvent == XAIE_EVENT_INVALID){
		XAieLib_print("Error: Invalid Event id\n");
		return XAIE_INVALID_ARGS;
	}

	/* Checking for valid Counter */
	if(Counter >= PerfMod->MaxCounterVal){
		XAieLib_print("Error: Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Get offset address based on Counter */
	RegOffset = PerfMod->PerfCtrlBaseAddr +
				(Counter / 2U * PerfMod->PerfCtrlOffsetAdd);
	/* Compute mask for performance control register */
	FldMask = (PerfMod->Start.Mask | PerfMod->Stop.Mask) <<
				(PerfMod->StartStopShift * (Counter % 2U));
	/* Compute value to be written to the performance control register */
	FldVal = XAie_SetField(IntStartEvent,
		PerfMod->Start.Lsb + (PerfMod->StartStopShift * (Counter % 2U)),
		PerfMod->Start.Mask << (PerfMod->StartStopShift * (Counter % 2U)))|
		XAie_SetField(IntStopEvent,
		PerfMod->Stop.Lsb + (PerfMod->StartStopShift * (Counter % 2U)),
		PerfMod->Stop.Mask << (PerfMod->StartStopShift * (Counter % 2U)));

	u8 R = Loc.Row;
	u8 C = Loc.Col;

	/* Compute absolute address and write to register */
	RegAddr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, R ,C) + RegOffset;
	XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/* This API configures the control registers corresponding to the counter
*  with the reset event for the given range of tiles
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @param        Counter:Performance Counter
* @param        ResetEvent:Event that triggers reset to the counter
* @return       XAIE_OK on success
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterResetControlSet(XAie_DevInst *DevInst,
			XAie_PerfCounters Counter, XAie_LocType Loc,
						XAie_Events ResetEvent)
{
	AieRC RC;
	u32 ResetRegOffset, ResetFldMask;
	u64 ResetRegAddr, ResetFldVal;
	u8 TileType, IntResetEvent;
	const XAie_PerfMod *PerfMod;
	const XAie_EvntMod *EvntMod;

	XAie_LocRange Range = { Loc, Loc, { 1U, 1U } };

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	PerfMod = DevInst->DevProp.DevMod[TileType].PerfMod;
	EvntMod = DevInst->DevProp.DevMod[TileType].EvntMod;
	/*
	 * Checking if the counter is memory module counter of AIE tile
	 * XAie_PerfCounters Enum number corresponds to mem module - counter 4
	 * or 5, it is implied to be counter 0 or 1 of mem module of aie tile.
	 */
	if(Counter >= XAIE_MEMPERFCOUNTER_0){
		Counter -= XAIE_MEMPERFCOUNTER_0;
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[1];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[1];
	}

	/* check if the event passed as input is corresponding to the module */
	if(ResetEvent < EvntMod->EventMin || ResetEvent > EvntMod->EventMax){
		XAieLib_print("Error: Invalid Event id: %d\n", ResetEvent);
		return XAIE_INVALID_ARGS;
	}

	/* Subtract the module offset from event number */
	ResetEvent -= EvntMod->EventMin;

	/* Getting the true event number from the enum to array mapping */
	IntResetEvent = EvntMod->XAie_EventNumber[ResetEvent];

	/*checking for valid true event number */
	if(IntResetEvent == XAIE_EVENT_INVALID){
		XAieLib_print("Error: Invalid Event id: %d\n", ResetEvent);
		return XAIE_INVALID_ARGS;
	}

	/* Checking for valid Counter */
	if(Counter >= PerfMod->MaxCounterVal){
		XAieLib_print("Error: Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Get offset address based on Counter */
	ResetRegOffset = PerfMod->PerfCtrlResetBaseAddr;

	/* Compute mask for performance control register */
	ResetFldMask = PerfMod->Reset.Mask <<
					(PerfMod->ResetShift * (Counter));
	/* Compute value to be written to the performance control register */
	ResetFldVal = XAie_SetField(IntResetEvent,
		PerfMod->Reset.Lsb + (PerfMod->ResetShift * Counter),
		PerfMod->Reset.Mask << (PerfMod->ResetShift * Counter));

	u8 R = Loc.Row;
	u8 C = Loc.Col;

	/* Compute absolute address and write to register */
			ResetRegAddr = DevInst->BaseAddr +
				_XAie_GetTileAddr(DevInst, R ,C) + ResetRegOffset;
			XAieGbl_MaskWrite32(ResetRegAddr,
					ResetFldMask, ResetFldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/* This API sets the performance counter event value for the given tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of Tile
* @param        Counter:Performance Counter
* @param        CounterVal:Performance Counter Value
* @return       XAIE_OK on success
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterSet(XAie_DevInst *DevInst, XAie_LocType Loc,
				XAie_PerfCounters Counter, u32 CounterVal)
{
	AieRC RC;
	u32 CounterRegOffset;
	u64 CounterRegAddr;
	u8 TileType;
	const XAie_PerfMod *PerfMod;
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	PerfMod = DevInst->DevProp.DevMod[TileType].PerfMod;
	/*
	 * Checking if the counter is memory module counter of AIE tile
	 * XAie_PerfCounters Enum number corresponds to mem module - counter 4
	 * or 5, it is implied to be counter 0 or 1 of mem module of aie tile.
	 */
	if(Counter >= XAIE_MEMPERFCOUNTER_0){
		Counter -= XAIE_MEMPERFCOUNTER_0;
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[1];
	}

	/* Checking for valid Counter */
	if(Counter >= PerfMod->MaxCounterVal){
		XAieLib_print("Error: Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Get offset address based on Counter */
	CounterRegOffset = PerfMod->PerfCounterBaseAddr +
					((Counter)*PerfMod->PerfCounterOffsetAdd);

	u8 R = Loc.Row;
	u8 C = Loc.Col;
	/* Compute absolute address and write to register */
	CounterRegAddr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, R ,C) + CounterRegOffset;
	XAieGbl_Write32(CounterRegAddr,CounterVal);

	return XAIE_OK;
}
/*****************************************************************************/
/* This API sets the performance counter event value for the given tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @param        Counter:Performance Counter
* @param        EventVal:Event value to set
* @return       XAIE_OK on success
*
* @note
*
******************************************************************************/
AieRC XAie_PerfCounterEventValueSet(XAie_DevInst *DevInst, XAie_LocType Loc,
					XAie_PerfCounters Counter, u32 EventVal)
{
	AieRC RC;
	u32 CounterRegOffset;
	u64 CounterRegAddr;
	u8 TileType;
	const XAie_PerfMod *PerfMod;
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	PerfMod = DevInst->DevProp.DevMod[TileType].PerfMod;
	/*
	 * Checking if the counter is memory module counter of AIE tile
	 * XAie_PerfCounters Enum number corresponds to mem module - counter 4
	 * or 5, it is implied to be counter 0 or 1 of mem module of aie tile.
	 */
	if(Counter >= XAIE_MEMPERFCOUNTER_0){
		Counter -= XAIE_MEMPERFCOUNTER_0;
		PerfMod = &DevInst->DevProp.DevMod[TileType].PerfMod[1];
	}

	/* Checking for valid Counter */
	if(Counter >= PerfMod->MaxCounterVal){
		XAieLib_print("Error: Invalid Counter number: %d\n", Counter);
		return XAIE_INVALID_ARGS;
	}

	/* Get offset address based on Counter */
	CounterRegOffset = (PerfMod->PerfCounterEvtValBaseAddr) +
				((Counter)*PerfMod->PerfCounterOffsetAdd);

	u8 R = Loc.Row;
	u8 C = Loc.Col;

	/* Compute absolute address and write to register */
	CounterRegAddr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, R ,C) + CounterRegOffset;
	XAieGbl_Write32(CounterRegAddr,EventVal);

	return XAIE_OK;
}
