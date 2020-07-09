/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_clock.c
* @{
*
* This file contains routines for AIE timers
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 06/26/2020  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_clock.h"
#include "xaie_helper.h"

/*****************************************************************************/
/***************************** Macro Definitions *****************************/

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API sets the column clock control register. Its configuration affects
* (enable or disable) all tile's clock above the Shim tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE SHIM tile
* @param        Enable: XAIE_ENABLE to enable column global clock buffer,
*                       XAIE_DISABLE to disable.
*
* @return       none
*
* @note         It is not required to check the DevInst and the Loc tile type
*               as the caller function should provide the correct value.
*
******************************************************************************/
static void  _XAie_PmSetColumnClockBuffer(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Enable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimClkBufCntr *ClkBufCntr;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ClkBufCntr = PlIfMod->ClkBufCntr;

	RegAddr = ClkBufCntr->RegOff +
			_XAie_GetTileAddr(DevInst, 0U, Loc.Col);
	FldVal = XAie_SetField(Enable, ClkBufCntr->ClkBufEnable.Lsb,
			ClkBufCntr->ClkBufEnable.Mask);

	XAie_Write32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/*
* This API enables clock for all tiles in the given device instance.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @param        Enable: XAIE_ENABLE to enable column global clock buffer,
*               XAIE_DISABLE to disable.
*
* @note         None
*
*******************************************************************************/
void _XAie_PmSetPartitionClock(XAie_DevInst *DevInst, u8 Enable)
{
	for(u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc;

		Loc = XAie_TileLoc(C, 0);
		_XAie_PmSetColumnClockBuffer(DevInst, Loc, Enable);
	}
}

/*****************************************************************************/
/*
* This API returns the clock reset enable status.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @return       clock reset value status.
*
* @note         None
*
*******************************************************************************/
u8 _XAie_CheckClockRstEnable(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType;
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimClkBufCntr *ClkBufCntr;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ClkBufCntr = PlIfMod->ClkBufCntr;

	return ClkBufCntr->RstEnable;
}
