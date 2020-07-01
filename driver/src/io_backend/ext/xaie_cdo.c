/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_cdo.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations for cdo backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   06/09/2020 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AIECDO__ /* AIE simulator */

#include "main_rts.h"
#include "cdo_rts.h"

#endif

#include "xaie_cdo.h"
#include "xaie_io.h"

/****************************** Type Definitions *****************************/
typedef struct {
	u64 BaseAddr;
} XAie_CdoIO;

/************************** Variable Definitions *****************************/
const XAie_Backend CdoBackend =
{
	.Type = XAIE_IO_BACKEND_CDO,
	.Ops.Init = XAie_CdoIO_Init,
	.Ops.Finish = XAie_CdoIO_Finish,
	.Ops.Write32 = XAie_CdoIO_Write32,
	.Ops.Read32 = XAie_CdoIO_Read32,
	.Ops.MaskWrite32 = XAie_CdoIO_MaskWrite32,
	.Ops.MaskPoll = XAie_CdoIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_CdoIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_CdoIO_BlockSet32,
	.Ops.CmdWrite = XAie_CdoIO_CmdWrite,
};

/************************** Function Definitions *****************************/
#ifdef __AIECDO__

/*****************************************************************************/
/**
*
* This is the memory IO function to free the global IO instance
*
* @param	IOInst: IO Instance pointer.
*
* @return	None.
*
* @note		The global IO instance is a singleton and freed when
* the reference count reaches a zero.
*
*******************************************************************************/
AieRC XAie_CdoIO_Finish(void *IOInst)
{
	free(IOInst);
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize the global IO instance
*
* @param	DevInst: Device instance pointer.
*
* @return	XAIE_OK on success. Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_CdoIO_Init(XAie_DevInst *DevInst)
{
	XAie_CdoIO *IOInst;

	IOInst = (XAie_CdoIO *)malloc(sizeof(*IOInst));
	if(IOInst == NULL) {
		XAieLib_print("Error: Memory allocation failed\n");
		return XAIE_ERR;
	}

	IOInst->BaseAddr = DevInst->BaseAddr;
	DevInst->IOInst = IOInst;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAie_CdoIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_CdoIO *CdoIOInst = (XAie_CdoIO *)IOInst;

	cdo_Write32(CdoIOInst->BaseAddr + RegOff, Value);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
*
* @return	32-bit read value.
*
* @note		None.
*
*******************************************************************************/
u32 XAie_CdoIO_Read32(void *IOInst, u64 RegOff)
{
	/* no-op */
	return 0;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write masked 32bit data to the specified
* address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAie_CdoIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	XAie_CdoIO *CdoIOInst = (XAie_CdoIO *)IOInst;
	cdo_MaskWrite32(CdoIOInst->BaseAddr + RegOff, Mask, Value);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask poll an address for a value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit value to poll for
* @param	TimeOutUs: Timeout in micro seconds.
*
* @return	XAIELIB_SUCCESS or XAIELIB_FAILURE.
*
* @note		None.
*
*******************************************************************************/
u32 XAie_CdoIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	XAie_CdoIO *CdoIOInst = (XAie_CdoIO *)IOInst;
	/* Round up to msec */
	cdo_MaskPoll(CdoIOInst->BaseAddr + RegOff, Mask, Value,
			(TimeOutUs + 999) / 1000);
	return XAIELIB_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write a block of data to aie.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to the data buffer.
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAie_CdoIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i++) {
		XAie_CdoIO_Write32(IOInst, RegOff + i * 4U, *Data);
		Data++;
	}
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize a chunk of aie address space with
* a specified value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Data to initialize a chunk of aie address space..
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAie_CdoIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i++)
		XAie_CdoIO_Write32(IOInst, RegOff+ i * 4U, Data);
}

#else

AieRC XAie_CdoIO_Finish(void *IOInst)
{
	/* no-op */
	return XAIE_OK;
}

AieRC XAie_CdoIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	XAieLib_print("WARNING: Driver is not compiled with cdo generation "
			"backend (__AIECDO__).IO Operations will result in "
			"no-ops\n");
	return XAIE_INVALID_BACKEND;
}

void XAie_CdoIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	/* no-op */
}

u32 XAie_CdoIO_Read32(void *IOInst, u64 RegOff)
{
	/* no-op */
	return 0;
}

void XAie_CdoIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	/* no-op */
}

u32 XAie_CdoIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	/* no-op */
	return XAIELIB_FAILURE;
}

void XAie_CdoIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	/* no-op */
}

void XAie_CdoIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
}

#endif /* __AIECDO__ */

void XAie_CdoIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
		u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
}

/** @} */
