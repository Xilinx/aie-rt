/******************************************************************************
* Copyright (C) 2023 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/***************************** Include Files *********************************/
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif // _XOPEN_SOURCE

#ifdef __linux__
#include <pthread.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_io_common.h"
#include "xaie_io_privilege.h"
#include "xaie_npi.h"

/****************************** Type Definitions *****************************/
typedef struct {
	u64 BaseAddr;
	u64 NpiBaseAddr;
	char *AmdAirAddress;
	char *AmdAirValue;
} XAie_AmdAirIO;

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
static AieRC XAie_AmdAirIO_Finish(void *IOInst)
{
	XAie_AmdAirIO *DevInst = (XAie_AmdAirIO *)IOInst;
	free(DevInst->AmdAirAddress);
	free(DevInst->AmdAirValue);
	free(DevInst);

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
static AieRC XAie_AmdAirIO_Init(XAie_DevInst *DevInst)
{
	XAie_AmdAirIO *IOInst;

	IOInst = (XAie_AmdAirIO *)malloc(sizeof(*IOInst));
	if(IOInst == NULL) {
		XAIE_ERROR("Memory allocation failed\n");
		return XAIE_ERR;
	}

	// recover sysfs path
	if (!DevInst->IOInst) {
		XAIE_ERROR("sysfs path not present!\n");
		return XAIE_ERR;
	}

	IOInst->BaseAddr = DevInst->BaseAddr;
	IOInst->NpiBaseAddr = XAIE_NPI_BASEADDR;

	IOInst->AmdAirAddress = malloc(strlen((const char *)DevInst->IOInst) + strlen("/address") + 1);
	sprintf(IOInst->AmdAirAddress, "%s/address", (const char *)DevInst->IOInst);

	IOInst->AmdAirValue = malloc(strlen((const char *)DevInst->IOInst) + strlen("/value") + 1);
	sprintf(IOInst->AmdAirValue, "%s/value", (const char *)DevInst->IOInst);

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
* @param	Value: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
static AieRC XAie_AmdAirIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	char buf[20];
	XAie_AmdAirIO *AmdAirIOInst = (XAie_AmdAirIO *)IOInst;

	XAIE_DBG("W: 0x%lx, 0x%x\n", RegOff, Value);

	int address = open(AmdAirIOInst->AmdAirAddress, O_WRONLY);
	if (address == -1) {
		XAIE_ERROR("Error opening %s\n", AmdAirIOInst->AmdAirAddress);
		return XAIE_ERR;
	}
	sprintf(buf, "0x%lx", RegOff);
	write(address, buf, strlen(buf) + 1);
	close(address);

	int value = open(AmdAirIOInst->AmdAirValue, O_WRONLY);
	if (value == -1) {
		XAIE_ERROR("Error opening %s\n", AmdAirIOInst->AmdAirValue);
		return XAIE_ERR;
	}
	sprintf(buf, "0x%x", Value);
	write(value, buf, strlen(buf) + 1);
	close(value);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to store the 32 bit value
*
* @return	XAIE_OK on success.
*
* @note		None.
*
*******************************************************************************/
static AieRC XAie_AmdAirIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	char buf[20];
	XAie_AmdAirIO *AmdAirIOInst = (XAie_AmdAirIO *)IOInst;

	int address = open(AmdAirIOInst->AmdAirAddress, O_WRONLY);
	if (address == -1) {
		XAIE_ERROR("Error opening %s\n", AmdAirIOInst->AmdAirAddress);
		return XAIE_ERR;
	}
	sprintf(buf, "0x%lx", RegOff);
	write(address, buf, strlen(buf) + 1);
	close(address);

	int value = open(AmdAirIOInst->AmdAirValue, O_RDONLY);
	if (value == -1) {
		XAIE_ERROR("Error opening %s\n", AmdAirIOInst->AmdAirValue);
		return XAIE_ERR;
	}
	read(value, buf, strlen(buf) + 1);
	close(value);
	*Data = strtoul(buf, 0, 0);

	XAIE_DBG("R: 0x%lx = 0x%x\n", RegOff, *Data);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* Write a value to a register by first clearing some bits from the register.
* The bits are cleared by reading the current value and applying the mask.
* Then the value is updated and written back to the register.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Offset of the register to update.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
static AieRC XAie_AmdAirIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	u32 RegVal;

	AieRC rc = XAie_AmdAirIO_Read32(IOInst, RegOff, &RegVal);
	if (rc != XAIE_OK) {
		XAIE_ERROR("Failed to perform Read32 during MaskWrite32 operation\n");
		return rc;
  }

	RegVal &= ~Mask;
	return XAie_AmdAirIO_Write32(IOInst, RegOff, (RegVal | Value));
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
* @return	XAIE_ERR.
*
* @note		None.
*
*******************************************************************************/
static AieRC XAie_AmdAirIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	u32 RegVal;
	u32 MinTimeOutUs = 200;
	AieRC Ret = XAIE_ERR;

	XAIE_DBG("MP: 0x%lx, 0x%x, 0x%x, %u\n", RegOff, Mask, Value, TimeOutUs);

	do {
		XAie_AmdAirIO_Read32(IOInst, RegOff, &RegVal);
		if((RegVal & Mask) == Value) {
			Ret = XAIE_OK;
			break;
		}

		if (TimeOutUs) {
			usleep(MinTimeOutUs);
			TimeOutUs -= (TimeOutUs > MinTimeOutUs ? MinTimeOutUs : TimeOutUs);
		}
	} while (TimeOutUs);

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
static AieRC XAie_AmdAirIO_BlockWrite32(void *IOInst, u64 RegOff,
		const u32 *Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i ++) {
		XAie_AmdAirIO_Write32(IOInst, RegOff + i * 4U, *Data);
		Data++;
	}

	return XAIE_OK;
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
static AieRC XAie_AmdAirIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data,
		u32 Size)
{
	for(u32 i = 0U; i < Size; i++)
		XAie_AmdAirIO_Write32(IOInst, RegOff+ i * 4U, Data);

	return XAIE_OK;
}

static AieRC XAie_AmdAirIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	XAIE_DBG("%s not implemented\n", __func__);

	/* no-op */
	(void)IOInst;
	(void)Col;
	(void)Row;
	(void)Command;
	(void)CmdWd0;
	(void)CmdWd1;
	(void)CmdStr;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the function to write to AI engine NPI registers
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to write.
* @param	RegVal: Register value to write
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
static void _XAie_AmdAirIO_NpiWrite32(void *IOInst, u32 RegOff,
		u32 RegVal)
{
	UNUSED(IOInst);
	UNUSED(RegOff);
	UNUSED(RegVal);

	XAIE_DBG("NPIMW not implemented\n");
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask poll a NPI address for a value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit value to poll for
* @param	TimeOutUs: Timeout in micro seconds.
*
* @return	XAIE_OK.
*
* @note		None.
*
*******************************************************************************/
static AieRC _XAie_AmdAirIO_NpiMaskPoll(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, u32 TimeOutUs)
{
	UNUSED(IOInst);
	UNUSED(RegOff);
	UNUSED(Mask);
	UNUSED(Value);
	UNUSED(TimeOutUs);

	XAIE_DBG("NPIMP: 0x%lx, 0x%x, 0x%x, 0x%d\n", AmdAirIOInst->NpiBaseAddr + RegOff,
			Mask, Value, TimeOutUs);

	return XAIE_OK;
}


/*****************************************************************************/
/**
*
* This is the function to run backend operations
*
* @param	IOInst: IO instance pointer
* @param	DevInst: AI engine partition device instance
* @param	Op: Backend operation code
* @param	Arg: Backend operation argument
*
* @return	XAIE_OK for success and error code for failure.
*
* @note		None.
*
*******************************************************************************/
static AieRC XAie_AmdAirIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC = XAIE_OK;
	(void)IOInst;

	XAIE_DBG("%s %u\n", __func__, Op);
	switch(Op) {
		case XAIE_BACKEND_OP_NPIWR32:
		{
			XAie_BackendNpiWrReq *Req = Arg;

			_XAie_AmdAirIO_NpiWrite32(IOInst, Req->NpiRegOff,
					Req->Val);
			break;
		}
		case XAIE_BACKEND_OP_NPIMASKPOLL32:
		{
			XAie_BackendNpiMaskPollReq *Req = Arg;

			return _XAie_AmdAirIO_NpiMaskPoll(IOInst, Req->NpiRegOff,
					Req->Mask, Req->Val, Req->TimeOutUs);
		}
		case XAIE_BACKEND_OP_ASSERT_SHIMRST:
		{
			u8 RstEnable = (u8)((uintptr_t)Arg & 0xFF);

			_XAie_NpiSetShimReset(DevInst, RstEnable);
			break;
		}
		case XAIE_BACKEND_OP_SET_PROTREG:
		{
			RC = _XAie_NpiSetProtectedRegEnable(DevInst, Arg);
			break;
		}
		case XAIE_BACKEND_OP_CONFIG_SHIMDMABD:
		{
			XAie_ShimDmaBdArgs *BdArgs = (XAie_ShimDmaBdArgs *)Arg;
			for(u8 i = 0; i < BdArgs->NumBdWords; i++) {
				XAie_AmdAirIO_Write32(IOInst,
						BdArgs->Addr + i * 4,
						BdArgs->BdWords[i]);
			}
			break;
		}
		case XAIE_BACKEND_OP_REQUEST_TILES:
			return _XAie_PrivilegeRequestTiles(DevInst,
					(XAie_BackendTilesArray *)Arg);
		case XAIE_BACKEND_OP_REQUEST_RESOURCE:
			return _XAie_RequestRscCommon(DevInst, Arg);
		case XAIE_BACKEND_OP_RELEASE_RESOURCE:
			return _XAie_ReleaseRscCommon(Arg);
		case XAIE_BACKEND_OP_FREE_RESOURCE:
			return _XAie_FreeRscCommon(Arg);
		case XAIE_BACKEND_OP_REQUEST_ALLOCATED_RESOURCE:
			return _XAie_RequestAllocatedRscCommon(DevInst, Arg);
		case XAIE_BACKEND_OP_PARTITION_INITIALIZE:
			return _XAie_PrivilegeInitPart(DevInst,
					(XAie_PartInitOpts *)Arg);
		case XAIE_BACKEND_OP_PARTITION_TEARDOWN:
			return _XAie_PrivilegeTeardownPart(DevInst);
		case XAIE_BACKEND_OP_GET_RSC_STAT:
			return _XAie_GetRscStatCommon(DevInst, Arg);
		default:
			XAIE_ERROR("Backend doesn't support Op %u.\n", Op);
			RC = XAIE_FEATURE_NOT_SUPPORTED;
			break;
	}

	return RC;
}

/*****************************************************************************/
/**
*
* This is the memory function to allocate a memory
*
* @param	DevInst: Device Instance
* @param	Size: Size of the memory
* @param	Cache: Buffer to be cacheable or not
*
* @return	Pointer to the allocated memory instance.
*
* @note		Internal only.
*
*******************************************************************************/
static XAie_MemInst* XAie_AmdAirMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	XAie_MemInst *MemInst;

	XAIE_DBG("%s\n", __func__);
	MemInst = (XAie_MemInst *)malloc(sizeof(*MemInst));
	if(MemInst == NULL) {
		XAIE_ERROR("memory allocation failed\n");
		return NULL;
	}

	MemInst->VAddr = (void *)malloc(Size);
	if(MemInst->VAddr == NULL) {
		XAIE_ERROR("malloc failed\n");
		free(MemInst);
		return NULL;
	}
	MemInst->DevAddr = (u64)MemInst->VAddr;
	MemInst->Size = Size;
	MemInst->DevInst = DevInst;

	(void)Cache;
	XAIE_DBG("Cache attribute is ignored\n");

	return MemInst;
}

/*****************************************************************************/
/**
*
* This is the memory function to free the memory
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_AmdAirMemFree(XAie_MemInst *MemInst)
{
	XAIE_DBG("%s\n", __func__);
	free(MemInst->VAddr);
	free(MemInst);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to sync the memory for CPU
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_AmdAirMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	XAIE_DBG("Sync for CPU is no-op in debug mode\n");

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to sync the memory for CPU
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_AmdAirMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	XAIE_DBG("Sync for Dev is no-op in debug mode\n");

	return XAIE_OK;
}

static AieRC XAie_AmdAirMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	XAIE_DBG("Mem attach is no-op in debug mode\n");

	return XAIE_OK;
}

static AieRC XAie_AmdAirMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	XAIE_DBG("Mem detach is no-op in debug mode\n");

	return XAIE_OK;
}

static u64 XAie_AmdAirGetTid(void)
{
#ifdef __linux__
	return (u64)pthread_self();
#else
	return 0;
#endif
}

const XAie_Backend AmdAirBackend =
{
	.Type = XAIE_IO_BACKEND_AMDAIR,
	.Ops.Init = XAie_AmdAirIO_Init,
	.Ops.Finish = XAie_AmdAirIO_Finish,
	.Ops.Write32 = XAie_AmdAirIO_Write32,
	.Ops.Read32 = XAie_AmdAirIO_Read32,
	.Ops.MaskWrite32 = XAie_AmdAirIO_MaskWrite32,
	.Ops.MaskPoll = XAie_AmdAirIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_AmdAirIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_AmdAirIO_BlockSet32,
	.Ops.CmdWrite = XAie_AmdAirIO_CmdWrite,
	.Ops.RunOp = XAie_AmdAirIO_RunOp,
	.Ops.MemAllocate = XAie_AmdAirMemAllocate,
	.Ops.MemFree = XAie_AmdAirMemFree,
	.Ops.MemSyncForCPU = XAie_AmdAirMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_AmdAirMemSyncForDev,
	.Ops.MemAttach = XAie_AmdAirMemAttach,
	.Ops.MemDetach = XAie_AmdAirMemDetach,
	.Ops.GetTid = XAie_AmdAirGetTid,
	.Ops.SubmitTxn = NULL,
};

/** @} */
