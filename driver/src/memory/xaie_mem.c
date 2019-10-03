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
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_DataMemWrite(DevInst, Range, Addr, 1, Data);
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

	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_DataMemRead(DevInst, Range, Addr, 1, Data);
}

/*****************************************************************************/
/**
*
* This API writes a 32-bit value to the specified data memory location for a
* range of tiles.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	Addr: Address in data memory to write.
* @param	Data: Data to write to the address.
*
* @return	XAIE_OK on success and error code on failure
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_DataMemWrite(XAie_DevInst *DevInst, XAie_LocRange Range, u32 Addr,
		u32 Len, u32 Data)
{
	AieRC RC = XAIE_OK;
	u64 RegAddr;
	const XAie_MemMod *MemMod;
	u8 TileType;
	u32 RangeLen;

	if(DevInst == XAIE_NULL) {
		RC = XAIE_INVALID_ARGS;
		XAieLib_print("Error %d: Invalid Device Instance\n", RC);
		return RC;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		RC = XAIE_INVALID_RANGE;
		XAieLib_print("Error %d: Invalid Device Range\n", RC);
		return RC;
	}

	RangeLen = _XAie_GetLenFromRange(Range);
	if(RangeLen != Len) {
		RC = XAIE_ERR_OUTOFBOUND;
		XAieLib_print("Error %d: AIE array index out of bound\n", RC);
		return RC;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)){
		RC = XAIE_INVALID_TILE;
		XAieLib_print("Error %d: Invalid Tile Type\n", RC);
		return RC;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		RC = XAIE_INVALID_RANGE;
		XAieLib_print("Error %d: Range has different Tile Types\n", RC);
		return RC;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;

	if(Addr + Len >= MemMod->Size) {
		RC = XAIE_INVALID_DATA_MEM_ADDR;
		XAieLib_print("Error %d: Address out of range\n", RC);
		return RC;
	}

	for(u8 R = Range.Start.Row; R <= Range.End.Row; R += Range.Stride.Row) {
		for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

			RegAddr = DevInst->BaseAddr +
				_XAie_GetTileAddr(DevInst, R, C) +
				MemMod->MemAddr + Addr;

			XAieGbl_Write32(RegAddr, Data);
		}
	}

	return RC;
}


/*****************************************************************************/
/**
*
* This API reads 32-bit value from the specified data memory location for a
* range of tiles. The memory to store the values read from memory is provided
* by the application layer.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	Addr: Address in data memory to write.
* @param	Data: Pointer to store 32-bit value read from memory.
*
* @return	XAIE_OK on success and error code on failure
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_DataMemRead(XAie_DevInst *DevInst, XAie_LocRange Range, u32 Addr, u32 Len,
		u32 *Data)
{
	AieRC RC = XAIE_OK;
	u64 RegAddr;
	const XAie_MemMod *MemMod;
	u8 TileType;
	u32 RangeLen;
	u32 Count;

	if((DevInst == XAIE_NULL) || (Data == XAIE_NULL)) {
		RC = XAIE_INVALID_ARGS;
		XAieLib_print("Error %d: Invalid Device Instance\n", RC);
		return RC;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		RC = XAIE_INVALID_RANGE;
		XAieLib_print("Error %d: Invalid Device Range\n", RC);
		return RC;
	}

	RangeLen = _XAie_GetLenFromRange(Range);
	if(RangeLen != Len) {
		RC = XAIE_ERR_OUTOFBOUND;
		XAieLib_print("Error %d: AIE array index out of bound\n", RC);
		return RC;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)){
		RC = XAIE_INVALID_TILE;
		XAieLib_print("Error %d: Invalid Tile Type\n", RC);
		return RC;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		RC = XAIE_INVALID_RANGE;
		XAieLib_print("Error %d: Range has different Tile Types\n", RC);
		return RC;
	}

	MemMod = DevInst->DevProp.DevMod[TileType].MemMod;

	if(Addr + Len >= MemMod->Size) {
		RC = XAIE_INVALID_DATA_MEM_ADDR;
		XAieLib_print("Error %d: Address out of range\n");
		return RC;
	}

	Count = 0;
	for(u8 R = Range.Start.Row; R <= Range.End.Row; R += Range.Stride.Row) {
		for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

			RegAddr = DevInst->BaseAddr +
				_XAie_GetTileAddr(DevInst, R, C) +
				MemMod->MemAddr + Addr;

			Data[Count] = XAieGbl_Read32(RegAddr);
			Count++;
		}
	}

	return RC;
}

/** @} */
