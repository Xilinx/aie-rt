/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_io.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   06/09/2020 Initial creation.
* 1.1   Tejus   06/10/2020 Add helper function to get backend pointer.
* </pre>
*
******************************************************************************/
#ifndef XAIE_IO_H
#define XAIE_IO_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"

/****************************** Type Definitions *****************************/
/*
 * Typdef to capture all the backend IO operations
 * Init        : Backend specific initialization function. Init should attach
 *               private data to DevInst which is later used by other ops.
 * Finish      : Backend specific IO finish function. Backend specific cleanup
 *               should be part of this function.
 * Write32     : IO operation to write 32-bit data.
 * Read32      : IO operation to read 32-bit data.
 * MaskWrite32 : IO operation to write masked 32-bit data.
 * MaskPoll    : IO operation to mask poll an address for a value.
 * BlockWrite32: IO operation to write a block of data at 32-bit granularity.
 * BlockSet32  : IO operation to initialize a chunk of aie address space with a
 *               a specified value at 32-bit granularity.
 * CmdWrite32  : This IO operation is required only in simulation mode. Other
 *               backends should have a no-op.
 */
typedef struct XAie_BackendOps {
	AieRC (*Init)(XAie_DevInst *DevInst);
	AieRC (*Finish)(void *IOInst);
	void (*Write32)(void *IOInst, u64 RegOff, u32 Value);
	u32 (*Read32)(void *IOInst,  u64 RegOff);
	void (*MaskWrite32)(void *IOInst, u64 RegOff, u32 Mask, u32 Value);
	u32 (*MaskPoll)(void *IOInst, u64 RegOff, u32 Mask, u32 Value, u32 TimeOutUs);
	void (*BlockWrite32)(void *IOInst, u64 RegOff, u32 *Data, u32 Size);
	void (*BlockSet32)(void *IOInst, u64 RegOff, u32 Data, u32 Size);
	void (*CmdWrite)(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
			u32 CmdWd1, const char *CmdStr);
} XAie_BackendOps;

/* Typedef to capture all backend inforamation */
typedef struct XAie_Backend {
	XAie_BackendType Type;
	XAie_BackendOps Ops;
} XAie_Backend;

/************************** Function Prototypes  *****************************/
AieRC XAie_IOInit(XAie_DevInst *DevInst);
const XAie_Backend* _XAie_GetBackendPtr(XAie_BackendType Backend);

#endif	/* End of protection macro */

/** @} */
