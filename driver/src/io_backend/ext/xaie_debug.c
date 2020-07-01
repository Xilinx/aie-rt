/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_debug.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations for debug.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   06/29/2020 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>

#include "xaie_debug.h"
#include "xaie_io.h"

/****************************** Type Definitions *****************************/
typedef struct {
	u64 BaseAddr;
} XAie_DebugIO;

/************************** Variable Definitions *****************************/
const XAie_Backend DebugBackend =
{
	.Type = XAIE_IO_BACKEND_DEBUG,
	.Ops.Init = XAie_DebugIO_Init,
	.Ops.Finish = XAie_DebugIO_Finish,
	.Ops.Write32 = XAie_DebugIO_Write32,
	.Ops.Read32 = XAie_DebugIO_Read32,
	.Ops.MaskWrite32 = XAie_DebugIO_MaskWrite32,
	.Ops.MaskPoll = XAie_DebugIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_DebugIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_DebugIO_BlockSet32,
	.Ops.CmdWrite = XAie_DebugIO_CmdWrite,
};

/************************** Function Definitions *****************************/
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
AieRC XAie_DebugIO_Finish(void *IOInst)
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
AieRC XAie_DebugIO_Init(XAie_DevInst *DevInst)
{
	XAie_DebugIO *IOInst;

	IOInst = (XAie_DebugIO *)malloc(sizeof(*IOInst));
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
void XAie_DebugIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_DebugIO *DebugIOInst = (XAie_DebugIO *)IOInst;

	printf("W: 0x%lx, 0x%x\n", DebugIOInst->BaseAddr + RegOff, Value);
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
u32 XAie_DebugIO_Read32(void *IOInst, u64 RegOff)
{
	XAie_DebugIO *DebugIOInst = (XAie_DebugIO *)IOInst;

	printf("R: 0x%lx, 0x%x\n", DebugIOInst->BaseAddr + RegOff, 0);

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
void XAie_DebugIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	XAie_DebugIO *DebugIOInst = (XAie_DebugIO *)IOInst;

	printf("MW: 0x%lx, 0x%x, 0x%x\n", DebugIOInst->BaseAddr + RegOff, Mask,
			Value);
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
u32 XAie_DebugIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	XAie_DebugIO *DebugIOInst = (XAie_DebugIO *)IOInst;

	printf("MP: 0x%lx, 0x%x, 0x%x, 0x%d\n", DebugIOInst->BaseAddr + RegOff,
			Mask, Value, TimeOutUs);

	return XAIELIB_FAILURE;
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
void XAie_DebugIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i ++) {
		XAie_DebugIO_Write32(IOInst, RegOff + i * 4U, *Data);
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
void XAie_DebugIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i++)
		XAie_DebugIO_Write32(IOInst, RegOff+ i * 4U, Data);
}

void XAie_DebugIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
		u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
}

/** @} */
