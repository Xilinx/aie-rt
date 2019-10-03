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
* @file xaie_helper.h
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
* </pre>
*
******************************************************************************/
#ifndef XAIEHELPER_H
#define XAIEHELPER_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* Checks the validity of the Range of AIE Tiles
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @return	XAIE_OK on success and XAIE_INVALID_RANGE on failure
*
* @note		Internal API only.
*
******************************************************************************/
static inline AieRC _XAie_CheckLocRange(XAie_DevInst *DevInst,
		XAie_LocRange Range)
{
	if(Range.End.Row > DevInst->NumRows || Range.End.Col > DevInst->NumCols)
		return XAIE_INVALID_RANGE;
	if(Range.Start.Row > DevInst->NumRows || Range.Start.Col >
			DevInst->NumCols)
		return XAIE_INVALID_RANGE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* Calculates the Tile Address from Row, Col of the AIE array/partition
*
* @param	DevInst: Device Instance
* @param	R: Row
* @param	C: Column
* @return	TileAddr
*
* @note		Internal API only.
*
******************************************************************************/
static inline u64 _XAie_GetTileAddr(XAie_DevInst *DevInst, int R, int C)
{
	return (R << DevInst->DevProp.RowShift) | (C << DevInst->DevProp.ColShift);
}

/*****************************************************************************/
/**
*
* Calculates number of AIE Tiles from Range
*
* @param	R: Range of AIE Tiles
* @return	Number of AIE Tiles
*
* @note		Internal API only.
*
******************************************************************************/
static inline u32 _XAie_GetLenFromRange(XAie_LocRange R)
{
	return ((R.End.Row - R.Start.Row + 1 + (R.Stride.Row - 1)) / R.Stride.Row) *
		((R.End.Col - R.Start.Col + 1 + (R.Stride.Col - 1)) / R.Stride.Col);
}

/*****************************************************************************/
/**
*
* Checks of the lock descriptor is valid or not
*
* @param	Lock: Lock descriptor
* @return	XAIE_OK if valid or XAIE_ERR if invalid
*
* @note		Internal API only.
*
******************************************************************************/
static inline AieRC _XAie_IsValidLock(XAie_LockDesc Lock, u8 LockMax)
{
	return (Lock.Id > LockMax ? XAIE_ERR : XAIE_OK);
}


u8 _XAie_GetTileType(XAie_DevInst *DevInst, XAie_LocRange Range);
AieRC _XAie_CheckRangeTileType(XAie_DevInst *DevInst, XAie_LocRange Range);

#endif		/* end of protection macro */
/** @} */
