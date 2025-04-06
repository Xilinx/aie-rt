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

/** @} */
