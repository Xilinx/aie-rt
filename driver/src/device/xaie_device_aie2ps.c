/******************************************************************************
* Copyright (C) 2023 AMD.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_device_aie2p.c
* @{
*
* This file contains the apis for device specific operations of aie2p ipu.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Keerthanna   01/25/2023  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaie_helper_internal.h"
#include "xaie2psgbl_params.h"

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the function used to get the tile type for a given device instance
* and tile location.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	TileType (AIETILE/MEMTILE/SHIMPL/SHIMNOC on success and MAX on
*		error)
*
* @note		Internal API only. This API returns tile type based on all
*		tiles on row 0 being shim noc tiles.
*
******************************************************************************/
u8 _XAie2PS_GetTTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{

	if(Loc.Col >= DevInst->NumCols) {
		XAIE_ERROR("Invalid column: %d\n", Loc.Col);
		return XAIEGBL_TILE_TYPE_MAX;
	}

	if(Loc.Row == 0U) {
		return XAIEGBL_TILE_TYPE_SHIMNOC;
	} else if(Loc.Row >= DevInst->MemTileRowStart &&
			(Loc.Row < (DevInst->MemTileRowStart +
				     DevInst->MemTileNumRows))) {
		return XAIEGBL_TILE_TYPE_MEMTILE;
	} else if (Loc.Row >= DevInst->AieTileRowStart &&
			(Loc.Row < (DevInst->AieTileRowStart +
				     DevInst->AieTileNumRows))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAIE_ERROR("Cannot find Tile Type for [%d, %d]\n", Loc.Col, Loc.Row);

	return XAIEGBL_TILE_TYPE_MAX;
}

AieRC _XAie2PS_SetAxiMMIsolation(XAie_DevInst* DevInst, u8 IsolationFlags)
{
	AieRC RC = XAIE_OK;
	u64 RegAddr;
	u32 RegVal;
	u8 TileType;
	static const XAie_AxiMMTileCtrlMod *AxiMMTileCtrlMod;

	for(u8 Col = 0; Col < DevInst->NumCols; Col++) {
		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst,
				XAie_TileLoc(Col, 0U));
		if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC &&
				TileType != XAIEGBL_TILE_TYPE_SHIMPL) {
			continue;
		}
		AxiMMTileCtrlMod = DevInst->DevProp.DevMod[TileType].AxiMMTileCtrlMod;
		RegVal = 0U;

		if(IsolationFlags == XAIE_INIT_ISOLATION) {
			if(Col == 0U) {
				RegVal |= AxiMMTileCtrlMod->AxiMMIsolateWest.Mask;
			}
			if(Col == (u8)(DevInst->NumCols - 1U)) {
				RegVal |= AxiMMTileCtrlMod->AxiMMIsolateEast.Mask;
			}
		}

		if(Col == 0U && (IsolationFlags & XAIE_INIT_WEST_ISOLATION)) {
			RegVal |= AxiMMTileCtrlMod->AxiMMIsolateWest.Mask;
		}
		if(Col == (u8)(DevInst->NumCols - 1U) && (IsolationFlags & XAIE_INIT_EAST_ISOLATION)) {
			RegVal |= AxiMMTileCtrlMod->AxiMMIsolateEast.Mask;
		}

		RegAddr = XAie_GetTileAddr(DevInst, 0U, Col) +
			AxiMMTileCtrlMod->TileCtrlAxiMMRegOff;
		RC = XAie_Write32(DevInst, RegAddr, RegVal);
	}
	return RC;
}

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
* @return       XAIE_OK for success, and error code for failure.
*
* @note         It is not required to check the DevInst and the Loc tile type
*               as the caller function should provide the correct value.
*               It is internal function to this file
*
******************************************************************************/
static AieRC _XAie2PS_PmSetColumnClockBuffer(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Enable)
{
	AieRC RC;
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	XAie_LocType ShimLoc = XAie_TileLoc(Loc.Col, 0U);
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimClkBufCntr *ClkBufCntr;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, ShimLoc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ClkBufCntr = PlIfMod->ClkBufCntr;

	RegAddr = ClkBufCntr->RegOff +
			XAie_GetTileAddr(DevInst, 0U, Loc.Col);
	FldVal = XAie_SetField(Enable, ClkBufCntr->ClkBufEnable.Lsb,
			ClkBufCntr->ClkBufEnable.Mask);

	RC = XAie_MaskWrite32(DevInst, RegAddr, ClkBufCntr->ClkBufEnable.Mask,
		FldVal);
	return RC;
}


/*****************************************************************************/
/**
* This API enable/disable the module clock control
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE SHIM tile
* @param        Enable: XAIE_ENABLE to enable shim clock buffer,
*                       XAIE_DISABLE to disable.
*
* @return       XAIE_OK for success, and error code for failure.
*
* @note         It is not required to check the DevInst and the Loc tile type
*               as the caller function should provide the correct value.
*               It is internal function to this file
*
******************************************************************************/
static AieRC _XAie2PS_PmSetShimClk(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Enable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	AieRC RC;
	XAie_LocType ShimLoc = XAie_TileLoc(Loc.Col, 0U);
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimModClkCntr0 *ModClkCntr0;
	const XAie_ShimModClkCntr1 *ModClkCntr1;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, ShimLoc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ModClkCntr0 = PlIfMod->ModClkCntr0;
	ModClkCntr1 = PlIfMod->ModClkCntr1;

	RegAddr = ModClkCntr0->RegOff +
			XAie_GetTileAddr(DevInst, 0U, Loc.Col);
	FldVal = XAie_SetField(Enable, ModClkCntr0->StrmSwClkEnable.Lsb,
			ModClkCntr0->StrmSwClkEnable.Mask);
	FldVal |= XAie_SetField(Enable, ModClkCntr0->PlIntClkEnable.Lsb,
			ModClkCntr0->PlIntClkEnable.Mask);
	FldVal |= XAie_SetField(Enable, ModClkCntr0->CteClkEnable.Lsb,
			ModClkCntr0->CteClkEnable.Mask);
	FldVal |= XAie_SetField(Enable, ModClkCntr0->UcClkEnable.Lsb,
			ModClkCntr0->UcClkEnable.Mask);

	RC = XAie_MaskWrite32(DevInst, RegAddr, XAIE2PSGBL_PL_MODULE_MODULE_CLOCK_CONTROL_0_MASK,
			FldVal);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to enable module clock control 0. Col: %u Row: %u\n",Loc.Col, Loc.Row);
		return RC;
	}

	RegAddr = ModClkCntr1->RegOff +
			XAie_GetTileAddr(DevInst, 0U, Loc.Col);
	FldVal = XAie_SetField(Enable, ModClkCntr1->NocModClkEnable.Lsb,
			ModClkCntr1->NocModClkEnable.Mask);

	RC = XAie_MaskWrite32(DevInst, RegAddr, XAIE2PSGBL_PL_MODULE_MODULE_CLOCK_CONTROL_1_MASK,
			FldVal);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to enable module clock control 1. Col: %u Row: %u\n", Loc.Col, Loc.Row);
		return RC;
	}

	return XAIE_OK;

}

/*****************************************************************************/
/**
* This API enables column clock and module clock control register for the
* requested tiles passed as argument to this API.
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Args: Backend column args
*
* @return       XAIE_OK on success, error code on failure
*
* @note		Internal only.
*
*******************************************************************************/
AieRC _XAie2PS_SetColumnClk(XAie_DevInst *DevInst, XAie_BackendColumnReq *Args)
{
	AieRC RC;

	u32 TileStatus, NumTiles;

	if((Args->NumCols == 0U) || (Args->StartCol >= DevInst->NumCols) ||
		((Args->StartCol + Args->NumCols) > DevInst->NumCols)) {
		XAIE_ERROR("Invalid Start Column/Numcols. Args->StartCol: %d Args->NumCols: %d \n",
			       Args->StartCol, Args->NumCols);
		return XAIE_ERR;
	}

	/*Enable the clock control register for shims*/
	for(u32 C = Args->StartCol; C < (Args->StartCol + Args->NumCols); C++) {
		XAie_LocType TileLoc = XAie_TileLoc(C, 1);

		RC = _XAie2PS_PmSetColumnClockBuffer(DevInst, TileLoc,
				Args->Enable);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable clock for column: %d\n",
					TileLoc.Col);
			return RC;
		}

		RC = _XAie2PS_PmSetShimClk(DevInst, TileLoc, Args->Enable);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to set module clock control.Col: %u Row: %u \n",
					TileLoc.Col, TileLoc.Row);
			return RC;
		}
	}

	TileStatus = _XAie_GetTileBitPosFromLoc(DevInst, XAie_TileLoc(Args->StartCol, 1));
	NumTiles =(u32)((DevInst->NumRows - 1U) * (Args->NumCols));

	if(Args->Enable) {
		/*
		 * Set bitmap from start column to Start+Number of columns
		 */
		_XAie_SetBitInBitmap(DevInst->DevOps->TilesInUse,
					TileStatus, NumTiles);
	} else {
		_XAie_ClrBitInBitmap(DevInst->DevOps->TilesInUse,
					TileStatus, NumTiles);
	}

	return XAIE_OK;
}


/** @} */
