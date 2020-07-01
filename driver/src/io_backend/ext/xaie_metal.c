/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_metal.c
* @{
*
* This file contains the low level layer IO interface
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Hyun    07/12/2018  Initial creation
* 1.1  Hyun    10/11/2018  Initialize the IO device for mem instance
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.3  Tejus   06/09/2020  Rename and import file from legacy driver.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#ifdef __AIEMETAL__

#include <metal/alloc.h>
#include <metal/device.h>
#include <metal/io.h>
#include <metal/irq.h>
#include <metal/list.h>
#include <metal/mutex.h>
#include <metal/shmem.h>
#include <metal/shmem-provider.h>
#include <metal/utilities.h>

#endif

#include "xaie_io.h"
#include "xaie_metal.h"

/***************************** Macro Definitions *****************************/
/****************************** Type Definitions *****************************/
#ifdef __AIEMETAL__

typedef struct XAie_MetalIO {
	struct metal_device *device;	/**< libmetal device */
	struct metal_io_region *io;	/**< libmetal io region */
	u64 io_base;			/**< libmetal io region base */
} XAie_MetalIO;

#endif /* __AIEMETAL__ */

/************************** Variable Definitions *****************************/
const XAie_Backend MetalBackend =
{
	.Type = XAIE_IO_BACKEND_METAL,
	.Ops.Init = XAie_MetalIO_Init,
	.Ops.Finish = XAie_MetalIO_Finish,
	.Ops.Write32 = XAie_MetalIO_Write32,
	.Ops.Read32 = XAie_MetalIO_Read32,
	.Ops.MaskWrite32 = XAie_MetalIO_MaskWrite32,
	.Ops.MaskPoll = XAie_MetalIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_MetalIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_MetalIO_BlockSet32,
	.Ops.CmdWrite = XAie_MetalIO_CmdWrite,
};

/************************** Function Definitions *****************************/
#ifdef __AIEMETAL__

/*****************************************************************************/
/**
*
* This is the memory IO function to free the global IO instance
*
* @param	IOInst: IO Instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The global IO instance is a singleton and freed when
* the reference count reaches a zero.
*
*******************************************************************************/
AieRC XAie_MetalIO_Finish(void *IOInst)
{
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)IOInst;

	metal_device_close(MetalIOInst->device);
	metal_free_memory(IOInst);
	metal_finish();

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize the global IO instance
*
* @param	None.
*
* @return	None.
*
* @note		The global IO instance is a singleton and any further attempt
* to initialize just increments the reference count.
*
*******************************************************************************/
AieRC XAie_MetalIO_Init(XAie_DevInst *DevInst)
{
	XAie_MetalIO *MetalIOInst;
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	int ret;

	ret = metal_init(&init_param);
	if(ret) {
		XAieLib_print("failed to metal_init %d\n", ret);
		return XAIE_ERR;
	}

	MetalIOInst = (XAie_MetalIO *)metal_allocate_memory(
			sizeof(*MetalIOInst));
	if(MetalIOInst == NULL) {
		XAieLib_print("Error: Memory allocation failed\n");
		goto finish;
	}


	ret = metal_device_open("platform", "xilinx-aiengine",
			&MetalIOInst->device);
	if(ret) {
		XAieLib_print("failed to metal_device_open\n");
		goto free_mem;
	}

	MetalIOInst->io = metal_device_io_region(MetalIOInst->device, 0);
	if(!MetalIOInst->io) {
		XAieLib_print("failed to metal_device_io_region\n");
		goto close;
	}

	MetalIOInst->io_base = metal_io_phys(MetalIOInst->io, 0);

	DevInst->IOInst = (void *)MetalIOInst;

	return XAIE_OK;

close:
	metal_device_close(MetalIOInst->device);

free_mem:
	metal_free_memory(MetalIOInst);

finish:
	metal_finish();
	return XAIE_ERR;
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
u32 XAie_MetalIO_Read32(void *IOInst, u64 RegOff)
{
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)IOInst;

	return metal_io_read32(MetalIOInst->io, RegOff);
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
void XAie_MetalIO_Write32(void *IOInst, u64 RegOff, u32 Data)
{
	XAie_MetalIO *MetalIOInst = (XAie_MetalIO *)IOInst;

	metal_io_write32(MetalIOInst->io, RegOff, Data);
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
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAie_MetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Data)
{
	u32 RegVal;

	RegVal = XAie_MetalIO_Read32(IOInst, RegOff);
	RegVal &= ~Mask;
	RegVal |= Data;
	XAie_MetalIO_Write32(IOInst, RegOff, RegVal);
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
u32 XAie_MetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
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
		if((XAie_MetalIO_Read32(IOInst, RegOff) & Mask) == Value) {
			Ret = XAIELIB_SUCCESS;
			break;
		}
		XAieLib_usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	if((Ret == XAIELIB_FAILURE) &&
			((XAie_MetalIO_Read32(IOInst, RegOff) & Mask) ==
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
void XAie_MetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	for(u32 i = 0; i < Size; i++) {
		XAie_MetalIO_Write32(IOInst, RegOff + i * 4U, *Data);
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
void XAie_MetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	for(u32 i = 0; i < Size; i++) {
		XAie_MetalIO_Write32(IOInst, RegOff + i * 4U, Data);
	}
}

#else

AieRC XAie_MetalIO_Finish(void *IOInst)
{
	/* no-op */
	return XAIE_OK;
}

AieRC XAie_MetalIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	XAieLib_print("WARNING: Driver is not compiled with Libmetal backend "
			"(__AIEMETAL__). IO Operations will result in no-ops."
			"\n");
	return XAIE_INVALID_BACKEND;
}

u32 XAie_MetalIO_Read32(void *IOInst, u64 RegOff)
{
	/* no-op */
	return 0;
}

void XAie_MetalIO_Write32(void *IOInst, u64 RegOff, u32 Data)
{
	/* no-op */
}

void XAie_MetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Data)
{
	/* no-op */
}

u32 XAie_MetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	/* no-op */
	return XAIELIB_FAILURE;
}

void XAie_MetalIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	/* no-op */
}

void XAie_MetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
}

#endif /* __AIEMETAL__ */

void XAie_MetalIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
		u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
}

/** @} */
