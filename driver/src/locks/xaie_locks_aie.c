/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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
* @file xaie_locks_aie.c
* @{
*
* This file contains routines for AIE locks.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/17/2020  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaie_locks.h"
#include "xaiegbl_defs.h"
/************************** Constant Definitions *****************************/
#define XAIE_LOCK_WITH_VALUE_OFF	0x20
#define XAIE_LOCK_RESULT_SUCCESS	1U
#define XAIE_LOCK_RESULT_LSB		0x0
#define XAIE_LOCK_RESULT_MASK		0x1

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to acquire the specified lock with or without value. Lock
* acquired without value if LockVal is (XAIE_LOCK_WTIH_NO_VALUE). Lock without
* value can be acquired for AIE only. This API can be blocking or non-blocking
* based on the TimeOut value. If the TimeOut is 0us, the API behaves in a
* non-blocking fashion and returns immediately after the first lock acquire
* request. If TimeOut > 0, then the API is a blocking call and will issue lock
* acquire request until the acquire is successful or it times out, whichever
* occurs first.
*
* @param	DevInst: Device Instance
* @param	LockMod: Internal lock module data structure.
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
* @param	TimeOut: Timeout value for which the acquire request needs to be
*		repeated. Value in usecs.
*
* @return	XAIE_OK if Lock Acquired, else XAIE_LOCK_RESULT_FAILED.
*
* @note 	Internal API for AIE. This API should not be called directly.
*		It is invoked only using the function pointer part of the lock
*		module data structure.
*
******************************************************************************/
AieRC _XAie_LockAcquire(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 TimeOut)
{
	u64 RegAddr;
	u32 RegOff;

	if(Lock.LockVal == XAIE_LOCK_WITH_NO_VALUE) {
		RegOff = LockMod->BaseAddr +
			(Lock.LockId * LockMod->LockIdOff) + LockMod->RelAcqOff;
	} else {
		RegOff = LockMod->BaseAddr + (Lock.LockId * LockMod->LockIdOff) +
			LockMod->RelAcqOff + XAIE_LOCK_WITH_VALUE_OFF +
			(Lock.LockVal * LockMod->LockValOff);
	}

	RegAddr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	if(XAieGbl_MaskPoll(RegAddr, XAIE_LOCK_RESULT_MASK,
				(XAIE_LOCK_RESULT_SUCCESS <<
				 XAIE_LOCK_RESULT_LSB), TimeOut)) {

		return XAIE_LOCK_RESULT_FAILED;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to release the specified lock with or without value. Lock is
* released without value if LockVal is (XAIE_LOCK_WITH_NO_VALUE). Lock without
* value can be released for AIE only. This API can be blocking or non-blocking
* based on the TimeOut value. If the TimeOut is 0us, the API behaves in a
* non-blocking fashion and returns immediately after the first lock release
* request. If TimeOut > 0, then the API is a blocking call and will issue lock
* release request until the release is successful or it times out, whichever
* occurs first.
*
* @param	DevInst: Device Instance
* @param	LockMod: Internal lock module data structure.
* @param	Loc: Location of AIE Tile
* @param	Lock: Lock data structure with LockId and LockValue.
* @param	TimeOut: Timeout value for which the release request needs to be
*		repeated. Value in usecs.
*
* @return	XAIE_OK if Lock Release, else XAIE_LOCK_RESULT_FAILED.
*
* @note 	Internal API for AIE. This API should not be called directly.
*		It is invoked only using the function pointer part of the lock
*		module data structure.
*
******************************************************************************/
AieRC _XAie_LockRelease(XAie_DevInst *DevInst, const XAie_LockMod *LockMod,
		XAie_LocType Loc, XAie_Lock Lock, u32 TimeOut)
{
	u64 RegAddr;
	u32 RegOff;

	if(Lock.LockVal == XAIE_LOCK_WITH_NO_VALUE) {
		RegOff = LockMod->BaseAddr + (Lock.LockId * LockMod->LockIdOff);
	} else {
		RegOff = LockMod->BaseAddr +
			(Lock.LockId * LockMod->LockIdOff) +
			XAIE_LOCK_WITH_VALUE_OFF +
			(Lock.LockVal * LockMod->LockValOff);
	}

	RegAddr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	if(XAieGbl_MaskPoll(RegAddr, XAIE_LOCK_RESULT_MASK,
				(XAIE_LOCK_RESULT_SUCCESS <<
				 XAIE_LOCK_RESULT_LSB), TimeOut)) {

		return XAIE_LOCK_RESULT_FAILED;
	}

	return XAIE_OK;
}

/** @} */
