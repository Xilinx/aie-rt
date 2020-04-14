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
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)){
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;
	if(Addr >= MemMod->Size) {
		XAieLib_print("Error: Address out of range\n");
		return XAIE_INVALID_DATA_MEM_ADDR;
	}

	RegAddr = DevInst->BaseAddr + MemMod->MemAddr + Addr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	XAieGbl_Write32(RegAddr, Data);

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
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)){
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;
	if(Addr >= MemMod->Size) {
		XAieLib_print("Error: Address out of range\n");
		return XAIE_INVALID_DATA_MEM_ADDR;
	}

	RegAddr = DevInst->BaseAddr + MemMod->MemAddr + Addr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	*Data = XAieGbl_Read32(RegAddr);

	return XAIE_OK;
}

/** @} */
