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
* @file xaie_helper.c
* @{
*
* This file contains inline helper functions for AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   09/24/2019  Fix range check logic for shim row
* 1.2   Tejus   01/04/2020  Cleanup error messages
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"

/************************** Variable Definitions *****************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the function used to get the tile type for a given device instance
* and range
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @return	TileType (AIETILE/MEMTILE/SHIMPL/SHIMNOC on success and MAX on
		error)
*
* @note		Internal API only. Returns based on start of the range only.
*
******************************************************************************/
u8 _XAie_GetTileType(XAie_DevInst *DevInst, XAie_LocRange Range)
{
	u8 ColType;
	u8 StartRow = Range.Start.Row;
	u8 StartCol = Range.Start.Col;

	if(StartRow == 0) {
		ColType = StartCol % 4;
		if((ColType == 0) || (ColType == 1)) {
			return XAIEGBL_TILE_TYPE_SHIMPL;
		}

		return XAIEGBL_TILE_TYPE_SHIMNOC;

	} else if(StartRow >= DevInst->MemTileRowStart &&
			(StartRow < (DevInst->MemTileRowStart +
				     DevInst->MemTileNumRows))) {
		return XAIEGBL_TILE_TYPE_MEMTILE;
	} else if (StartRow >= DevInst->AieTileRowStart &&
			(StartRow < (DevInst->AieTileRowStart +
				     DevInst->AieTileNumRows))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAieLib_print("Error: Cannot find Tile Type\n");

	return XAIEGBL_TILE_TYPE_MAX;
}

/*****************************************************************************/
/**
*
* This is the function checks if all the tile types in the given range are the
* same.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @return	XAIE_OK on success and error code on failure
*
* @note		Internal API only. For Shim Row, the range operation is
*		supported with stride 4 only.
*
******************************************************************************/
AieRC _XAie_CheckRangeTileType(XAie_DevInst *DevInst, XAie_LocRange Range)
{
	u8 StartColType;
	u8 EndColType;
	u8 StartRow = Range.Start.Row;
	u8 StartCol = Range.Start.Col;
	u8 EndRow = Range.End.Row;
	u8 EndCol = Range.End.Col;

	/* Return AIE_OK if range is one tile */
	if((StartRow == EndRow) && (StartCol == EndCol))
		return XAIE_OK;

	if(StartRow == 0U) {
		if(EndRow != 0U) {
			XAieLib_print("Invalid shim row range\n");
			return XAIE_INVALID_RANGE;
		}
		/*
		 * TODO: For Shim Tiles, Support Range access only when the
		 * stride is 1 or a multiple of 4. Further combinations can be
		 * supported in future if required..
		 */

		/* Checks for valid range when stride is 1 */
		if((Range.Stride.Col == 1U) && (EndCol - StartCol == 1U)) {
			StartColType = StartCol % 4U;
			EndColType = EndCol % 4U;
			/*
			 * For SHIMPL, sum should be 1 and for SHIMNOC, sum
			 * should be 5
			 */
			if(((StartColType + EndColType) != 1U) &&
					((StartColType + EndColType) != 5U)) {
				XAieLib_print("Range has different shim tiles\n");
				return XAIE_INVALID_RANGE;
			}

			return XAIE_OK;
		}
		if((Range.Stride.Col % 4U) != 0U) {
			XAieLib_print("Invalid stride for shim row range\n");
			return XAIE_INVALID_RANGE;
		}
	} else if(StartRow >= DevInst->MemTileRowStart &&
			(StartRow < (DevInst->MemTileRowStart +
				     DevInst->MemTileNumRows))) {
		/* Range check for MemTile Rows*/
		if(Range.End.Row >= Range.Start.Row + DevInst->MemTileNumRows){
			XAieLib_print("Invalid MemTile Range\n");
			return XAIE_INVALID_RANGE;
		}
	} else if (StartRow >= DevInst->AieTileRowStart &&
			(StartRow < (DevInst->AieTileRowStart +
				     DevInst->AieTileNumRows))) {
		/* Range check for Aie Tile Rows*/
		if(Range.End.Row >= Range.Start.Row + DevInst->AieTileNumRows){
			XAieLib_print("Invalid AieTile Range\n");
			return XAIEGBL_TILE_TYPE_AIETILE;
		}
	} else {
		XAieLib_print("Error: Cannot find Tile Type\n");
		return XAIE_INVALID_RANGE;
	}

	return XAIE_OK;
}

/** @} */
