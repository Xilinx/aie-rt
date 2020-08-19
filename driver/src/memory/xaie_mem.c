/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_ss.h
* @{
*
* This file contains routines for AIE stream switch
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus	01/04/2020  Cleanup error messages
* 1.2   Tejus   03/20/2020  Reorder range apis
* 1.3   Tejus   03/20/2020  Make internal functions static
* 1.4   Tejus   04/13/2020  Remove range apis and change to single tile apis
* 1.5   Tejus   06/10/2020  Switch to new io backend apis.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_mem.h"


/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API writes a 32-bit value to the specified data memory location for
* the selected tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Addr: Address in data memory to write.
* @param	Data: 32-bit value to be written.
*
* @return	XAIE_OK on success and error code on failure
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_DataMemWrWord(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 Addr, u32 Data)
{
	u64 RegAddr;
	const XAie_MemMod *MemMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)){
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;
	if(Addr >= MemMod->Size) {
		XAIE_ERROR("Address out of range\n");
		return XAIE_INVALID_DATA_MEM_ADDR;
	}

	RegAddr = MemMod->MemAddr + Addr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	XAie_Write32(DevInst, RegAddr, Data);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API read a 32-bit value from the specified data memory location for
* the selected tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Addr: Address in data memory to write.
* @param	Data: Pointer to store 32-bit value read from memory.
*
* @return	XAIE_OK on success and error code on failure
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_DataMemRdWord(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 Addr, u32 *Data)
{
	u64 RegAddr;
	const XAie_MemMod *MemMod;
	u8 TileType;

	if((DevInst == XAIE_NULL) || (Data == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)){
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;
	if(Addr >= MemMod->Size) {
		XAIE_ERROR("Address out of range\n");
		return XAIE_INVALID_DATA_MEM_ADDR;
	}

	RegAddr = MemMod->MemAddr + Addr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	*Data = XAie_Read32(DevInst, RegAddr);

	return XAIE_OK;
}

/** @} */
