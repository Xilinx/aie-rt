/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_baremetal.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations for baremetal backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   07/04/2020 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#ifdef __AIEBAREMETAL__

#include "sleep.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"

#endif

#include "xaie_baremetal.h"
#include "xaie_io.h"

/****************************** Type Definitions *****************************/
typedef struct {
	u64 BaseAddr;
} XAie_BaremetalIO;

/************************** Variable Definitions *****************************/
static XAie_BaremetalIO BaremetalIO;

const XAie_Backend BaremetalBackend =
{
	.Type = XAIE_IO_BACKEND_BAREMETAL,
	.Ops.Init = XAie_BaremetalIO_Init,
	.Ops.Finish = XAie_BaremetalIO_Finish,
	.Ops.Write32 = XAie_BaremetalIO_Write32,
	.Ops.Read32 = XAie_BaremetalIO_Read32,
	.Ops.MaskWrite32 = XAie_BaremetalIO_MaskWrite32,
	.Ops.MaskPoll = XAie_BaremetalIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_BaremetalIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_BaremetalIO_BlockSet32,
	.Ops.CmdWrite = XAie_BaremetalIO_CmdWrite,
	.Ops.RunOp = XAie_BaremetalIO_RunOp,
};

/************************** Function Definitions *****************************/
#ifdef __AIEBAREMETAL__

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
AieRC XAie_BaremetalIO_Finish(void *IOInst)
{
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
AieRC XAie_BaremetalIO_Init(XAie_DevInst *DevInst)
{
	XAie_BaremetalIO *IOInst = &BaremetalIO;

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
void XAie_BaremetalIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_BaremetalIO *BaremetalIOInst = (XAie_BaremetalIO *)IOInst;

	Xil_Out32(BaremetalIOInst->BaseAddr + RegOff, Value);
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
u32 XAie_BaremetalIO_Read32(void *IOInst, u64 RegOff)
{
	XAie_BaremetalIO *BaremetalIOInst = (XAie_BaremetalIO *)IOInst;

	return Xil_In32(BaremetalIOInst->BaseAddr + RegOff);
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
void XAie_BaremetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	u32 RegVal = XAie_BaremetalIO_Read32(IOInst, RegOff);

	RegVal &= ~Mask;
	RegVal |= Value;

	XAie_BaremetalIO_Write32(IOInst, RegOff, RegVal);
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
u32 XAie_BaremetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	u32 Ret = XAIELIB_FAILURE;
	u32 Count, MinTimeOutUs;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	MinTimeOutUs = 200;
	Count = ((u64)TimeOutUs + MinTimeOutUs - 1) / MinTimeOutUs;

	while (Count > 0U) {
		if((XAie_BaremetalIO_Read32(IOInst, RegOff) & Mask) == Value) {
			Ret = XAIELIB_SUCCESS;
			break;
		}
		usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	if((Ret == XAIELIB_FAILURE) &&
			((XAie_BaremetalIO_Read32(IOInst, RegOff) & Mask) ==
			 Value)) {
		Ret = XAIELIB_SUCCESS;
	}

	return Ret;
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
void XAie_BaremetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data,
		u32 Size)
{
	for(u32 i = 0U; i < Size; i++) {
		XAie_BaremetalIO_Write32(IOInst, RegOff + i * 4U, *Data);
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
void XAie_BaremetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i++)
		XAie_BaremetalIO_Write32(IOInst, RegOff+ i * 4U, Data);
}

#else

AieRC XAie_BaremetalIO_Finish(void *IOInst)
{
	/* no-op */
	return XAIE_OK;
}

AieRC XAie_BaremetalIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	XAieLib_print("WARNING: Driver is not compiled with baremetal "
			"backend (__AIEBAREMETAL__).IO Operations will result "
			"in no-ops\n");
	return XAIE_INVALID_BACKEND;
}

void XAie_BaremetalIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	/* no-op */
}

u32 XAie_BaremetalIO_Read32(void *IOInst, u64 RegOff)
{
	/* no-op */
	return 0;
}

void XAie_BaremetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	/* no-op */
}

u32 XAie_BaremetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	/* no-op */
	return XAIELIB_FAILURE;
}

void XAie_BaremetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data,
		u32 Size)
{
	/* no-op */
}

void XAie_BaremetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
}

#endif /* __AIEBAREMETAL__ */

void XAie_BaremetalIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
}

AieRC XAie_BaremetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}
/** @} */
