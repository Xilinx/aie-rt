/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_timer.c
* @{
*
* This file contains routines for AIE timers
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 04/06/2020  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaie_helper.h"
#include "xaie_timer.h"
#include "xaiegbl.h"

/*****************************************************************************/
/***************************** Macro Definitions *****************************/
#define XAIE_TIMER_32BIT_SHIFT		32U

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API sets the timer trigger events value. Timer low event will be
* generated if the timer low reaches the specified low event value. Timer high
* event will be generated if the timer high reaches the specified high event
* value.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Pl or Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
*
* @param	LowEventValue: Value to set for the timer to trigger timer low
*                              event.
* @param	HighEventValue: Value to set for the timer to trigger timer
*                               high event.
*
* @return	XAIE_OK on success.
* 		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid
*
* @note
*
*******************************************************************************/
AieRC XAie_SetTimerTrigEventVal(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u32 LowEventValue, u32 HighEventValue)
{
	u64 RegAddr;
	u8 TileType, RC;
	const XAie_TimerMod *TimerMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		XAieLib_print("Error: Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[0U];
	}

	else {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[Module];
	}

	/* Set up Timer low event value */
	RegAddr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row ,Loc.Col) +
		TimerMod->TrigEventLowValOff;
	XAieGbl_Write32(RegAddr, LowEventValue);

	/* Set up Timer high event value */
	RegAddr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row ,Loc.Col) +
		TimerMod->TrigEventHighValOff;
	XAieGbl_Write32(RegAddr, HighEventValue);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API resets the timer
*
* @param	DevInst - Device Instance.
* @param	Loc - Location of tile.
* @param	Module - Module of the tile
*			 For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			 For Pl or Shim tile - XAIE_PL_MOD,
*			 For Mem tile - XAIE_MEM_MOD.
*
* @return	XAIE_OK on success.
*		XAIE_INVALID_ARGS if any argument is invalid
*		XAIE_INVALID_TILE if tile type from Loc is invalid

* @note
*
*******************************************************************************/
AieRC XAie_ResetTimer(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module)
{
	u32 RegVal, Mask;
	u64 RegAddr;
	u8 TileType, RC;
	const XAie_TimerMod *TimerMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		XAieLib_print("Error: Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[0U];
	}

	else {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[Module];
	}

	RegAddr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		TimerMod->CtrlOff;
	Mask = TimerMod->CtrlReset.Mask;
	RegVal = XAie_SetField(XAIE_RESETENABLE, TimerMod->CtrlReset.Lsb, Mask);
	XAieGbl_MaskWrite32(RegAddr, Mask, RegVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API sets the timer reset event. The timer will reset when the event
* is raised.
*
* @param	DevInst - Device Instance.
* @param	Loc - Location of tile.
* @param	Module - Module of the tile
*                         For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                         For Pl or Shim tile - XAIE_PL_MOD,
*                         For Mem tile - XAIE_MEM_MOD.
* @param	Event - Reset event.
* @param	Reset - Indicate if reset is also required in this call.
*                       (XAIE_RESETENABLE, XAIE_RESETDISABLE)
*
* @return	XAIE_OK on success.
*		XAIE_INVALID_ARGS if any argument is invalid$
* 		XAIE_INVALID_TILE if tile type from Loc is invalid$
*
* @note
*
*******************************************************************************/
AieRC XAie_SetTimerResetEvent(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events Event,
		XAie_Reset Reset)
{
	u32 RegVal;
	u64 RegAddr;
	u8 TileType, IntEvent, RC;
	const XAie_TimerMod *TimerMod;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		XAieLib_print("Error: Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[0U];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	}

	else {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[Module];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	/* check if the event passed as input is corresponding to the module */
	if(Event < EvntMod->EventMin || Event > EvntMod->EventMax) {
		XAieLib_print("Error: Invalid Event id\n");
		return XAIE_INVALID_ARGS;
	}

	/* Subtract the module offset from event number */
	Event -= EvntMod->EventMin;

	/* Getting the true event number from the enum to array mapping */
	IntEvent = EvntMod->XAie_EventNumber[Event];

	/*checking for valid true event number */
	if(IntEvent == XAIE_EVENT_INVALID) {
		XAieLib_print("Error: Invalid Event id\n");
		return XAIE_INVALID_ARGS;
	}

	RegVal = XAie_SetField(IntEvent, TimerMod->CtrlResetEvent.Lsb,
			TimerMod->CtrlResetEvent.Mask);

	RegVal |= XAie_SetField(Reset, TimerMod->CtrlReset.Lsb,
			TimerMod->CtrlReset.Mask);

	RegAddr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		TimerMod->CtrlOff;
	XAieGbl_Write32(RegAddr, RegVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the current value of the Core module 64-bit timer.
*
* @param	DevInst - Device Instance.
* @param	Loc - Location of tile.
* @param	Module - Module of the tile
*                         For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*                         For Pl or Shim tile - XAIE_PL_MOD,
*                         For Mem tile - XAIE_MEM_MOD.
*
* @return	64-bit timer value.
*
* @note		None.
*
********************************************************************************/
u64 XAie_ReadTimer(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module)
{
	u32 CurValHigh, CurValLow;
	u64 CurVal;
	u8 TileType, RC;
	const XAie_TimerMod *TimerMod;

	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}
	 /* check for module and tiletype combination */
	RC = _XAie_CheckModule(DevInst, Loc, Module);
        if(RC != XAIE_OK) {
		XAieLib_print("Error: Invalid Module\n");
                return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD) {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[0U];
	}

	else {
		TimerMod = &DevInst->DevProp.DevMod[TileType].TimerMod[Module];
	}

	/* Read the timer high and low values before wait */
	CurValLow = XAieGbl_Read32(DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		TimerMod->LowOff);
	CurValHigh = XAieGbl_Read32(DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		TimerMod->HighOff);
	CurVal = ((u64)CurValHigh << XAIE_TIMER_32BIT_SHIFT) | CurValLow;

	return CurVal;
}
