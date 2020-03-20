/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* @file xaie_core.c
* @{
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus	01/04/2020  Cleanup error messages
* 1.2   Tejus   03/20/2020  Reorder functions
* 1.3   Tejus   03/20/2020  Make internal functions static
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_core.h"

/************************** Constant Definitions *****************************/
#define XAIETILE_CORE_STATUS_DEF_WAIT_USECS 500U

/************************** Function Definitions *****************************/
/*****************************************************************************/
/*
*
* This API writes to the Core control register of a range of tiles to enable
* and/or reset the AIE core. This API writes to the register after some basic
* checks. Any gracefullness required in enabling/disabling and/or resetting/
* unresetting the core are required to be handled by the application layer/
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	Enable:  Enable/Disable the core (1- Enable, 0-Disable).
* @param	Reset - Reset/Unreset the core (1-Reset,0-Unreset).
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC XAie_CoreControl(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 Enable, u8 Reset)
{
	u32 RegVal;
	const XAie_CoreMod *CoreMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Invalid Device Range\n");
		return XAIE_INVALID_RANGE;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Range has different Tile Types\n");
		return XAIE_INVALID_RANGE;
	}

	/* Get core module pointer from device instance */
	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;
	/* Setup register values that have to be wrriten across the range */
	RegVal = XAie_SetField(Enable, CoreMod->CoreCtrl->CtrlEn.Lsb,
			CoreMod->CoreCtrl->CtrlEn.Mask) |
		XAie_SetField(Reset, CoreMod->CoreCtrl->CtrlRst.Lsb,
				CoreMod->CoreCtrl->CtrlRst.Mask);

	for(u8 R = Range.Start.Row; R <= Range.End.Row; R += Range.Stride.Row) {
		for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

			u64 RegAddr;
			/* Compute register address based of R and C */
			RegAddr = DevInst->BaseAddr +
				_XAie_GetTileAddr(DevInst, R, C)
				+ CoreMod->CoreCtrl->RegOff;

			XAieGbl_Write32(RegAddr, RegVal);
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/*
*
* This API implements a blocking wait function to check the core status for a
* range of AIE Tiles for a given Mask and Value. API comes out of the wait loop
* when the status changes to the Mask, Value pair or the timeout elapses,
* whichever happens first.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	TimeOut: TimeOut in usecs. If set to 0, the default timeout will
*		be set to 500us. The TimeOut value passed is per tile.
* @param	Mask: Mask for the core status register.
* @param	Value: Value for the core status register.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API only.
*
******************************************************************************/
static AieRC XAie_CoreWaitStatus(XAie_DevInst *DevInst, XAie_LocRange Range,
		u32 TimeOut,u32 Mask, u32 Value)
{

	u64 RegAddr;
	const XAie_CoreMod *CoreMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Invalid Device Range\n");
		return XAIE_INVALID_RANGE;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Range has different Tile Types\n");
		return XAIE_INVALID_RANGE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;

	/* TimeOut passed by the user is per Core */
	if(TimeOut == 0) {
		/* Set timeout to default value */
		TimeOut = XAIETILE_CORE_STATUS_DEF_WAIT_USECS;
	}

	for(u8 R = Range.Start.Row; R <= Range.End.Row; R += Range.Stride.Row) {
		for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

			RegAddr = DevInst->BaseAddr +
				_XAie_GetTileAddr(DevInst, R, C)
				+ CoreMod->CoreSts->RegOff;
			if(XAieGbl_MaskPoll(RegAddr, Mask, Value, TimeOut) !=
					XAIE_SUCCESS) {
				XAieLib_print("Error: Status poll time out"\
						"for Col %d, Row %d\n", R, C);
				return XAIE_CORE_STATUS_TIMEOUT;
			} else {
				continue;
			}
		}
	}

	return XAIE_OK;

}

/*****************************************************************************/
/*
*
* This API writes to the Core control register of a range of tiles to enable
* and unreset the AIE core. This API writes to the register after some basic
* checks. Any gracefullness required in enabling/disabling and/or resetting/
* unresetting the core are required to be handled by the application layer.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC XAie_CoreEnableRange(XAie_DevInst *DevInst, XAie_LocRange Range)
{
	return XAie_CoreControl(DevInst, Range, XAIE_ENABLE, XAIE_DISABLE);
}
/*****************************************************************************/
/*
*
* This API writes to the Core control register of a range of tiles to disable
* and reset the AIE core. This API writes to the register after some basic
* checks. Any gracefullness required in enabling/disabling and/or resetting/
* unresetting the core are required to be handled by the application layer.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC XAie_CoreDisableRange(XAie_DevInst *DevInst, XAie_LocRange Range)
{
	return XAie_CoreControl(DevInst, Range, XAIE_DISABLE, XAIE_ENABLE);
}

/*****************************************************************************/
/*
*
* This API writes to the Core control register of a tile to disable
* and reset the AIE core. This API writes to the register after some basic
* checks. Any gracefullness required in enabling/disabling and/or resetting/
* unresetting the core are required to be handled by the application layer.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tiles
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreDisable(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_CoreDisableRange(DevInst, Range);
}

/*****************************************************************************/
/*
*
* This API writes to the Core control register of a tile to enable and unreset
* the AIE core. This API writes to the register after some basic checks. Any
* gracefullness required in enabling/disabling and/or resetting/unresetting
* the core are required to be handled by the application layer.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreEnable(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_CoreEnableRange(DevInst, Range);
}

/*****************************************************************************/
/*
*
* This API implements a blocking wait function to check the core to be in
* done state for a range of AIE tiles. API comes out of the loop when core
* status changes to done or the timeout elapses, whichever happens first.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	TimeOut: TimeOut in usecs. If set to 0, the default timeout will
*		be set to 500us. The TimeOut value passed is per tile.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC XAie_CoreWaitForDoneRange(XAie_DevInst *DevInst,
		XAie_LocRange Range, u32 TimeOut)
{
	const XAie_CoreMod *CoreMod;
	u32 Mask;
	u32 Value;
	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;
	Mask = CoreMod->CoreSts->Done.Mask;
	Value = 1 << CoreMod->CoreSts->Done.Lsb;
	return XAie_CoreWaitStatus(DevInst, Range, TimeOut, Mask, Value);
}

/*****************************************************************************/
/*
*
* This API implements a blocking wait function to check the core to be in
* disable state for a range of AIE tiles. API comes out of the loop when core
* status changes to disable or the timeout elapses, whichever happens first.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	TimeOut: TimeOut in usecs. If set to 0, the default timeout will
*		be set to 500us. The TimeOut value passed is per tile.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC XAie_CoreWaitForDisableRange(XAie_DevInst *DevInst,
		XAie_LocRange Range, u32 TimeOut)
{
	const XAie_CoreMod *CoreMod;
	u32 Mask;
	u32 Value;
	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;
	Mask = CoreMod->CoreSts->En.Mask;
	Value = 0 << CoreMod->CoreSts->En.Lsb;
	return XAie_CoreWaitStatus(DevInst, Range, TimeOut, Mask, Value);
}

/*****************************************************************************/
/*
*
* This API implements a blocking wait function to check the core to be in
* done state for a AIE tile. API comes out of the loop when core status
* changes to done or the timeout elapses, whichever happens first.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	TimeOut: TimeOut in usecs. If set to 0, the default timeout will
*		be set to 500us. The TimeOut value passed is per tile.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 TimeOut)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_CoreWaitForDoneRange(DevInst, Range, TimeOut);
}

/*****************************************************************************/
/*
*
* This API implements a blocking wait function to check the core to be in
* disable state for a AIE tile. API comes out of the loop when core status
* changes to disable or the timeout elapses, whichever happens first.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	TimeOut: TimeOut in usecs. If set to 0, the default timeout will
*		be set to 500us. The TimeOut value passed is per tile.
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreWaitForDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 TimeOut)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_CoreWaitForDisableRange(DevInst, Range, TimeOut);
}

/** @} */
