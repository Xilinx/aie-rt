/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
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
#include <stdlib.h>

#ifdef __AIEBAREMETAL__

#ifdef XAIE_PROD
#include "pm_init.h"
#include "xpm_defs.h"
#endif
#include "sleep.h"
#include "xil_cache.h"
#include "xil_io.h"
#include "xil_types.h"
#include "xstatus.h"
#endif

#include "xaie_helper.h"
#include "xaie_helper_internal.h"
#include "xaie_io.h"
#include "xaie_io_common.h"
#include "xaie_io_privilege.h"
#include "xaie_npi.h"
#include "btree4.h"

#ifdef __AIEBAREMETAL__

/****************************** Type Definitions *****************************/
typedef struct {
	u64 BaseAddr;
	u64 NpiBaseAddr;
	struct btree4 btree;
} XAie_BaremetalIO;

/************************** Function Definitions *****************************/

static int XAie_BaremetalIO_MemInst_Compare(void *a, void *b)
{
	XAie_MemInst *MemA = (XAie_MemInst *)a;
	XAie_MemInst *MemB = (XAie_MemInst *)b;
	uint64_t VAddrA = (uint64_t)MemA->VAddr;
	uint64_t VAddrB = (uint64_t)MemB->VAddr;
	uint64_t VAddrBEnd = VAddrB + MemB->Size;

	if (VAddrA < VAddrB) {
		return -1;
	} else if (VAddrA >= VAddrBEnd) {
		return 1;
	}
	return 0;
}

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
* the reference count reaches a zero. Internal only.
*
*******************************************************************************/
static AieRC XAie_BaremetalIO_Finish(void *IOInst)
{
	XAie_BaremetalIO *Baremetal_IOInst = (XAie_BaremetalIO *)IOInst;

	if (Baremetal_IOInst->btree.root) {
		XAIE_ERROR("Trying to free IOInst while Meminsts exists.\n");
		return XAIE_ERR;
	}
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
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_BaremetalIO_Init(XAie_DevInst *DevInst)
{
	XAie_BaremetalIO *IOInst;
#ifdef XAIE_PROD
	static XIpiPsu IpiInst;
	int Ret;
#endif

	IOInst = (XAie_BaremetalIO *)malloc(sizeof(*IOInst));
	if(IOInst == NULL) {
		XAIE_ERROR("Baremetal backend init failed. failed to allocate memory\n");
		return XAIE_ERR;
	}

	IOInst->BaseAddr = DevInst->BaseAddr;
	IOInst->NpiBaseAddr = XAIE_NPI_BASEADDR;
	BTREE4_INIT(&IOInst->btree, XAie_BaremetalIO_MemInst_Compare);
	DevInst->IOInst = (void *)IOInst;

#if defined(XAIE_PROD)
	if (DevInst->DevProp.DevGen <= XAIE_DEV_GEN_AIEML) {
		DevInst->IsProd = 1U;

		Ret = XAie_PmInit(&IpiInst);
		if (Ret != XST_SUCCESS) {
			XAIE_ERROR("Failed to initialize PM\n");
			return Ret;
		}
	}
#endif

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
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_BaremetalIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_BaremetalIO *BaremetalIOInst = (XAie_BaremetalIO *)IOInst;

	Xil_Out32(BaremetalIOInst->BaseAddr + RegOff, Value);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the specified address
* using PLM.
*
* @param	IOInst: IO instance pointer
* @param	StartCol: Start column of the partition.
* @param	NumCols: Number of columns in the partition.
* @param	Ops: Operation ID to pass to PLM.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_BaremetalIO_PrivilegeWrite32(u32 StartCol,
						u32 NumCols, u32 Ops)
{
#if defined(XAIE_PROD)
	u32 Response;
	int Ret;

	Ret = XPm_DevIoctl(PM_DEV_AIE, IOCTL_AIE_OPS, (NumCols << 16)
			      | StartCol, Ops, &Response);
	if (Ret != XST_SUCCESS) {
		XAIE_ERROR("Failed to write to privileged register. NumCols: %u StartCol: %u PLM Op ID:%u\n", NumCols, StartCol, Ops);
		return XAIE_ERR;
	}
#endif

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
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_BaremetalIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	XAie_BaremetalIO *BaremetalIOInst = (XAie_BaremetalIO *)IOInst;

	*Data = Xil_In32(BaremetalIOInst->BaseAddr + RegOff);

	return XAIE_OK;
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
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_BaremetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	AieRC RC;
	u32 RegVal;

	RC = XAie_BaremetalIO_Read32(IOInst, RegOff, &RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	RegVal &= ~Mask;
	RegVal |= Value;

	return XAie_BaremetalIO_Write32(IOInst, RegOff, RegVal);
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
* @return	XAIE_OK or XAIE_ERR.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_BaremetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, u32 TimeOutUs)
{
	AieRC Ret = XAIE_ERR;
	u32 Count, MinTimeOutUs, RegVal;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	MinTimeOutUs = 200;
	Count = (TimeOutUs + MinTimeOutUs - 1U) / MinTimeOutUs;

	while (Count > 0U) {
		XAie_BaremetalIO_Read32(IOInst, RegOff, &RegVal);
		if((RegVal & Mask) == Value) {
			return XAIE_OK;
		}
		usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	XAie_BaremetalIO_Read32(IOInst, RegOff, &RegVal);
	if((RegVal & Mask) == Value) {
		Ret = XAIE_OK;
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
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_BaremetalIO_BlockWrite32(void *IOInst, u64 RegOff,
		const u32 *Data, u32 Size)
{
	for(u32 i = 0U; i < Size; i++) {
		XAie_BaremetalIO_Write32(IOInst, RegOff + (u64)(i * 4U), *Data);
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
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_BaremetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data,
		u32 Size)
{
	for(u32 i = 0U; i < Size; i++) {
		XAie_BaremetalIO_Write32(IOInst, RegOff+ (u64)(i * 4U), Data);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to allocate a memory
*
* @param	DevInst: Device Instance
* @param	Size: Size of the memory
* @param	Cache: Value from XAie_MemCacheProp enum
*
* @return	Pointer to the allocated memory instance.
*
* @note		Internal only.
*
*******************************************************************************/
static XAie_MemInst* XAie_BaremetalMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	XAie_MemInst *MemInst;
	XAie_BaremetalIO *IOInst = (XAie_BaremetalIO *)DevInst->IOInst;
	int Ret;

	(void)Cache;
	MemInst = (XAie_MemInst *)malloc(sizeof(*MemInst));
	if(MemInst == NULL) {
		XAIE_ERROR("memory allocation failed\n");
		return NULL;
	}

	MemInst->VAddr = (void *)aligned_alloc(16, Size);
	if(MemInst->VAddr == NULL) {
		XAIE_ERROR("malloc failed\n");
		free(MemInst);
		return NULL;
	}
	MemInst->DevAddr = (u64)(uintptr_t)MemInst->VAddr;
	MemInst->Size = Size;
	MemInst->DevInst = DevInst;
	Ret = btree4_insert(&IOInst->btree, MemInst);
	if (Ret)
		goto free_meminst;
	/*
	 * TODO: Cache is not handled at the moment for baremetal. The allocated
	 * memory is always cached.
	 */

	return MemInst;

free_meminst:
	free(MemInst->VAddr);
	free(MemInst);
	return NULL;
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
static AieRC XAie_BaremetalMemFree(XAie_MemInst *MemInst)
{
	XAie_BaremetalIO *IOInst = (XAie_BaremetalIO *)MemInst->DevInst->IOInst;
	btree4_delete(&IOInst->btree, MemInst);
	free(MemInst->VAddr);
	free(MemInst);

	return XAIE_OK;
}

static AieRC XAie_BaremetalMemFreeVAddr(XAie_DevInst *DevInst, void *VAddr)
{
	XAie_BaremetalIO *IOInst = (XAie_BaremetalIO *)DevInst->IOInst;
	XAie_MemInst MemInst;
	XAie_MemInst *Node;

	MemInst.VAddr = VAddr;
	MemInst.Size = 0;

	Node = btree4_search(&IOInst->btree, &MemInst);
	return XAie_MemFree(Node);
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
static AieRC XAie_BaremetalMemSyncForCPU(XAie_MemInst *MemInst)
{
	Xil_DCacheInvalidateRange((intptr_t)MemInst->VAddr,
			(intptr_t)MemInst->Size);

	return XAIE_OK;
}

static AieRC XAie_BaremetalMemSyncForCPUVAddr(XAie_DevInst *DevInst, void *VAddr,
					      uint64_t Size)
{
	(void)DevInst;
	Xil_DCacheInvalidateRange((intptr_t)VAddr, (intptr_t)Size);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to sync the memory for device
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_BaremetalMemSyncForDev(XAie_MemInst *MemInst)
{
	Xil_DCacheFlushRange((intptr_t)MemInst->VAddr,
			(intptr_t)MemInst->Size);

	return XAIE_OK;
}

static AieRC XAie_BaremetalMemSyncForDevVAddr(XAie_DevInst *DevInst, void *VAddr,
					 uint64_t Size)
{
	(void)DevInst;
	Xil_DCacheFlushRange((intptr_t)VAddr, (intptr_t)Size);

	return XAIE_OK;
}

static AieRC XAie_BaremetalMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_OK;
}

static AieRC XAie_BaremetalMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_OK;
}

static AieRC XAie_BaremetalMemGetDevAddrFromVAddr(XAie_DevInst *DevInst,
						  void *VAddr,
						  uint64_t *DevAddr)
{
	(void)DevInst;
	*DevAddr = (uint64_t)VAddr;
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the function to write 32 bit value to NPI register address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: NPI register offset
* @param	RegVal: Value to write to register
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_BaremetalIO_NpiWrite32(void *IOInst, u32 RegOff, u32 RegVal)
{
	XAie_BaremetalIO *BaremetalIOInst = (XAie_BaremetalIO *)IOInst;

	Xil_Out32(BaremetalIOInst->NpiBaseAddr + RegOff, RegVal);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified NPI
* address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to store the 32 bit value
*
* @return	XAIE_OK on success.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_BaremetalIO_NpiRead32(void *IOInst, u64 RegOff, u32 *Data)
{
	XAie_BaremetalIO *BaremetalIOInst = (XAie_BaremetalIO *)IOInst;

	*Data = Xil_In32(BaremetalIOInst->NpiBaseAddr + RegOff);

	return XAIE_OK;
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
* @return	XAIE_OK or XAIE_ERR.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_BaremetalIO_NpiMaskPoll(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, u32 TimeOutUs)
{
	AieRC Ret = XAIE_ERR;
	u32 Count, MinTimeOutUs, RegVal;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	MinTimeOutUs = 200;
	Count = (u32)(((u64)TimeOutUs + MinTimeOutUs - 1U) / MinTimeOutUs);

	while (Count > 0U) {
		_XAie_BaremetalIO_NpiRead32(IOInst, RegOff, &RegVal);
		if((RegVal & Mask) == Value) {
			return XAIE_OK;
		}
		usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	_XAie_BaremetalIO_NpiRead32(IOInst, RegOff, &RegVal);
	if((RegVal & Mask) == Value) {
		Ret = XAIE_OK;
	}

	return Ret;
}

/*****************************************************************************/
/**
* This API initializes the AI engine partition via PLM
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Opts: Initialization options
*
* @return       XAIE_OK on success, error code on failure
*
* @note		This operation does the following steps to initialize an AI
*		engine partition:
*		- Clock gate all columns
*		- Reset Columns
*		- Ungate all Columns
*		- Remove columns reset
*		- Reset shims
*		- Setup AXI MM not to return errors for AXI decode or slave
*		  errors, raise events instead.
*		- ungate all columns
*		- Setup partition isolation.
*		- zeroize memory if it is requested
*
*******************************************************************************/
static AieRC _XAie_BaremetalIO_PrivilegeInitPart(XAie_DevInst *DevInst,
						 XAie_PartInitOpts *Opts)
{
	AieRC RC = XAIE_OK;
#ifdef XAIE_PROD
	u32 OptFlags;

	if(Opts != NULL) {
		OptFlags = Opts->InitOpts;
	} else {
		OptFlags = XAIE_PART_INIT_OPT_DEFAULT;
	}

	if((OptFlags & XAIE_PART_INIT_OPT_COLUMN_RST) != 0U) {
		RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol,
				    DevInst->NumCols, AIE_OPS_COL_RST);
	}

	if((OptFlags & XAIE_PART_INIT_OPT_SHIM_RST) != 0U) {
		RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol,
					    DevInst->NumCols, AIE_OPS_SHIM_RST);
	}

	if((OptFlags & XAIE_PART_INIT_OPT_BLOCK_NOCAXIMMERR) != 0U) {
		RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol,
					    DevInst->NumCols,
					    AIE_OPS_ENB_AXI_MM_ERR_EVENT);
	}

	RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol,
				    DevInst->NumCols,
				    AIE_OPS_ENB_COL_CLK_BUFF);

	if ((OptFlags & XAIE_PART_INIT_OPT_ISOLATE) != 0U) {
		RC = DevInst->DevOps->SetPartIsolationAfterRst(DevInst, XAIE_INIT_ISOLATION);
		if(RC != XAIE_OK) {
			return RC;
		}
	}
	else {
		RC = DevInst->DevOps->SetPartIsolationAfterRst(DevInst, XAIE_CLEAR_ISOLATION);
		if(RC != XAIE_OK) {
			return RC;
		}
	}

	if ((OptFlags & XAIE_PART_INIT_OPT_ZEROIZEMEM) != 0U) {
		RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol,
					    DevInst->NumCols,
					    AIE_OPS_ALL_MEM_ZEROIZATION);
	}

	RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol, DevInst->NumCols,
				    AIE_OPS_SET_L2_CTRL_NPI_INTR);

	/* Enable only the tiles requested in Opts parameter */
	if(Opts != NULL) {
		XAie_BackendTilesArray TilesArray;

		TilesArray.NumTiles = Opts->NumUseTiles;
		TilesArray.Locs = Opts->Locs;

		RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_REQUEST_TILES,
		(void *)&TilesArray);

		if(RC != XAIE_OK) {
			return RC;
		}
	}

	/*
	 * This is a temporary workaround to unblock rel-v2023.1 and make
	 * XAie_PartitionInitialize() consistent with XAie_ResetPartition().
	 */
	if (DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE) {
		RC = _XAie_PmSetPartitionClock(DevInst, XAIE_DISABLE);
		if (RC != XAIE_OK) {
			return RC;
		}

		for(u32 C = 0; C < DevInst->NumCols; C++) {
			XAie_LocType Loc;
			u32 ColClockStatus;

			Loc = XAie_TileLoc(C, 1);
			ColClockStatus = _XAie_GetTileBitPosFromLoc(DevInst, Loc);
			_XAie_ClrBitInBitmap(DevInst->DevOps->TilesInUse,
				       ColClockStatus, DevInst->NumRows - 1);
		}
	}

#endif
	return RC;
}

/*****************************************************************************/
/**
* This API tears down the AI engine partition
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Opts: Initialization options
*
* @return       XAIE_OK on success, error code on failure
*
* @note		This operation does the following steps to initialize an AI
*		engine partition:
*		- Clock gate all columns
*		- Reset Columns
*		- Ungate all columns
*		- Reset shims
*		- Remove columns reset
*		- Ungate all columns
*		- Zeroize memories
*		- Clock gate all columns
*
*******************************************************************************/
static AieRC _XAie_BaremetalIO_PrivilegeTeardownPart(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;

#if defined(XAIE_PROD)

	RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol,
				    DevInst->NumCols, AIE_OPS_COL_RST);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Column reset failed!\n");
		return RC;
	}

	RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol,
				    DevInst->NumCols, AIE_OPS_SHIM_RST);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Shim reset failed!\n");
		return RC;
	}

	RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol,
				    DevInst->NumCols,
				    AIE_OPS_ENB_COL_CLK_BUFF);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Column Ungating failed!\n");
		return RC;
	}

	RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol,
				    DevInst->NumCols,
				    AIE_OPS_ALL_MEM_ZEROIZATION);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Memory Zeroization failed!\n");
		return RC;
	}


	if (DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE) {
		RC = _XAie_PmSetPartitionClock(DevInst, XAIE_DISABLE);
		if (RC != XAIE_OK) {
			return RC;
		}
	} else {
		RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol,
					    DevInst->NumCols,
					    AIE_OPS_DIS_COL_CLK_BUFF);
		if (RC != XAIE_OK) {
			XAIE_ERROR("Column Gating failed!\n");
		}
	}
#endif

	return RC;
}

AieRC _XAie_BaremetalIO_PrivilegeSetColumnClk(XAie_DevInst *DevInst,
					      XAie_BackendColumnReq *Args)
{
#if defined(XAIE_PROD)
	AieRC RC;

	u32 TileStatus, NumTiles, Ops;
	u32 PartEndCol = (u32)(DevInst->StartCol + DevInst->NumCols - 1U);

	if((Args->StartCol < DevInst->StartCol) || (Args->StartCol > PartEndCol) ||
			((Args->StartCol + Args->NumCols - 1U) > PartEndCol) ) {
		XAIE_ERROR("Invalid Start Column/Numcols. StartCol:%u NumCols:%u \n",Args->StartCol, Args->NumCols);
		return XAIE_ERR;
	}

	Ops = Args->Enable ? AIE_OPS_ENB_COL_CLK_BUFF: AIE_OPS_DIS_COL_CLK_BUFF;
	RC = _XAie_BaremetalIO_PrivilegeWrite32(Args->StartCol, Args->NumCols,Ops);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to enable clock for column. StartCol:%u NumCols:%u\n",Args->StartCol, Args->NumCols);
		return RC;
	}

	TileStatus = _XAie_GetTileBitPosFromLoc(DevInst,
			XAie_TileLoc((u8)Args->StartCol, 1));
	NumTiles =(u32)((DevInst->NumRows - 1U) * (Args->NumCols));

	if(Args->Enable) {
		/*
		 * Set bitmap from start column to Start+Number of columns
		 */
		_XAie_SetBitInBitmap(DevInst->DevOps->TilesInUse,
				TileStatus, NumTiles);
	} else {
		_XAie_ClrBitInBitmap(DevInst->DevOps->TilesInUse,
				TileStatus, NumTiles);
	}
#endif
	return XAIE_OK;
}

AieRC _XAie_BaremetalIO_PrivilegeRequestTiles(XAie_DevInst *DevInst,
					      XAie_BackendTilesArray *Args)
{
#if defined(XAIE_PROD)
	AieRC RC;
	u32 SetTileStatus;

	if(Args->Locs == NULL) {
		u32 NumTiles;
		XAie_LocType TileLoc = XAie_TileLoc(0, 1);
		NumTiles = (u32)((DevInst->NumRows - 1U) * (DevInst->NumCols));

		SetTileStatus = _XAie_GetTileBitPosFromLoc(DevInst, TileLoc);
		_XAie_SetBitInBitmap(DevInst->DevOps->TilesInUse, SetTileStatus,
				     NumTiles);

		return _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol, DevInst->NumCols,
							AIE_OPS_ENB_COL_CLK_BUFF);
	}

	/* Disbale all the column clock and enable only the requested column clock */
	RC = _XAie_BaremetalIO_PrivilegeWrite32(DevInst->StartCol, DevInst->NumCols,
						AIE_OPS_DIS_COL_CLK_BUFF);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to enable clock for column. StartCol:%u NumCol:%u\n",DevInst->StartCol,DevInst->NumCols);
		return RC;
	}

	/* Clear the TilesInuse bitmap to reflect the current status */
	for(u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc;
		u32 ColClockStatus;

		Loc = XAie_TileLoc((u8)C, 1U);
		ColClockStatus = _XAie_GetTileBitPosFromLoc(DevInst, Loc);
		_XAie_ClrBitInBitmap(DevInst->DevOps->TilesInUse,
				ColClockStatus, (u32)(DevInst->NumRows - 1U));
	}

	for(u32 i = 0; i < Args->NumTiles; i++) {
		u32 ColClockStatus;
		/*
	         * Shim rows are enabled by default, skip shim row
		 */
		if (Args->Locs[i].Row == 0U) {
			continue;
		}
		/*
		 * Check if column clock buffer is already enabled and continue
		 * Get bitmap position from first row after shim
		 */
		ColClockStatus = _XAie_GetTileBitPosFromLoc(DevInst,
				XAie_TileLoc(Args->Locs[i].Col, 1));
		if (CheckBit(DevInst->DevOps->TilesInUse, ColClockStatus)) {
			continue;
		}

		RC = _XAie_BaremetalIO_PrivilegeWrite32(Args->Locs[i].Col, 1U,
							AIE_OPS_ENB_COL_CLK_BUFF);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable clock for column. Col:%u\n",Args->Locs[i].Col);
			return RC;
		}

		/*
		 * Set bitmap for entire column, row 1 to last row.
		 * Shim row is already set, so use NumRows-1
		 */
		_XAie_SetBitInBitmap(DevInst->DevOps->TilesInUse,
			ColClockStatus, (u32)(DevInst->NumRows - 1U));
	}
#endif
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
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_BaremetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)DevInst;
	switch(Op) {
		case XAIE_BACKEND_OP_NPIWR32:
		{
			XAie_BackendNpiWrReq *Req = Arg;

			_XAie_BaremetalIO_NpiWrite32(IOInst, Req->NpiRegOff,
					Req->Val);
			break;
		}
		case XAIE_BACKEND_OP_NPIMASKPOLL32:
		{
			XAie_BackendNpiMaskPollReq *Req = Arg;

			return _XAie_BaremetalIO_NpiMaskPoll(IOInst,
					Req->NpiRegOff, Req->Mask, Req->Val,
					Req->TimeOutUs);
		}
		case XAIE_BACKEND_OP_CONFIG_SHIMDMABD:
		{
			XAie_ShimDmaBdArgs *BdArgs =
				(XAie_ShimDmaBdArgs *)Arg;

			XAie_BaremetalIO_BlockWrite32(IOInst,
				BdArgs->Addr, BdArgs->BdWords, BdArgs->NumBdWords);
			break;
		}
		case XAIE_BACKEND_OP_REQUEST_TILES:
			if (DevInst->IsProd == 1U &&
					DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
				return _XAie_BaremetalIO_PrivilegeRequestTiles(DevInst,
						(XAie_BackendTilesArray *)Arg);

			} else {
				return _XAie_PrivilegeRequestTiles(DevInst,
						(XAie_BackendTilesArray *)Arg);
			}
		case XAIE_BACKEND_OP_PARTITION_INITIALIZE:
			if (DevInst->IsProd == 1U) {
				return _XAie_BaremetalIO_PrivilegeInitPart(DevInst,
						(XAie_PartInitOpts *)Arg);
			} else {
				return _XAie_PrivilegeInitPart(DevInst,
					(XAie_PartInitOpts *)Arg);
			}
		case XAIE_BACKEND_OP_PARTITION_TEARDOWN:
			if (DevInst->IsProd == 1U) {
				return _XAie_BaremetalIO_PrivilegeTeardownPart(DevInst);
			} else {
				return _XAie_PrivilegeTeardownPart(DevInst);
			}
		case XAIE_BACKEND_OP_UPDATE_NPI_ADDR:
		{
			XAie_BaremetalIO *BaremetalIOInst =
				(XAie_BaremetalIO *)IOInst;
			BaremetalIOInst->NpiBaseAddr = *((u64 *)Arg);
			break;
		}
		case XAIE_BACKEND_OP_SET_COLUMN_CLOCK:
		{
			if (DevInst->IsProd == 1U && DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
				return _XAie_BaremetalIO_PrivilegeSetColumnClk(DevInst,
						(XAie_BackendColumnReq *)Arg);
			} else {
				return _XAie_PrivilegeSetColumnClk(DevInst,
						(XAie_BackendColumnReq *)Arg);
			}
		}
		default:
			XAIE_ERROR("Baremetal backend doesn't support operation"
					" %d\n", Op);
			return XAIE_FEATURE_NOT_SUPPORTED;
	}

	return XAIE_OK;
}

#else

static AieRC XAie_BaremetalIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

static AieRC XAie_BaremetalIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with baremetal "
			"backend (__AIEBAREMETAL__)\n");
	return XAIE_INVALID_BACKEND;
}

static AieRC XAie_BaremetalIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Value;

	return XAIE_ERR;
}

static AieRC XAie_BaremetalIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;

	return XAIE_ERR;
}

static AieRC XAie_BaremetalIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, u32 TimeOutUs)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalIO_BlockWrite32(void *IOInst, u64 RegOff,
		const u32 *Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_BaremetalIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data,
		u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static XAie_MemInst* XAie_BaremetalMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

static AieRC XAie_BaremetalMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalMemFreeVAddr(XAie_DevInst *DevInst, void *VAddr)
{
	(void)DevInst;
	(void)VAddr;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalMemSyncForCPUVAddr(XAie_DevInst *DevInst, void *VAddr,
					      uint64_t Size)
{
	(void)DevInst;
	(void)VAddr;
	(void)Size;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalMemSyncForDevVAddr(XAie_DevInst *DevInst, void *VAddr,
					      uint64_t Size)
{
	(void)DevInst;
	(void)VAddr;
	(void)Size;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalMemGetDevAddrFromVAddr(XAie_DevInst *DevInst,
						  void *VAddr,
						  uint64_t *DevAddr)
{
	(void)DevInst;
	(void)VAddr;
	(void)DevAddr;
	return XAIE_ERR;
}

static AieRC XAie_BaremetalIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

static AieRC _XAie_BaremetalIO_PrivilegeWrite32(u32 StartCol,
						u32 NumCols, u32 Ops)
{
	(void) StartCol;
	(void) NumCols;
	(void) Ops;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

#endif /* __AIEBAREMETAL__ */

static AieRC XAie_BaremetalIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
	(void)IOInst;
	(void)Col;
	(void)Row;
	(void)Command;
	(void)CmdWd0;
	(void)CmdWd1;
	(void)CmdStr;

	return XAIE_ERR;
}

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
	.Ops.PrivilegeWrite32 = _XAie_BaremetalIO_PrivilegeWrite32,
	.Ops.BlockSet32 = XAie_BaremetalIO_BlockSet32,
	.Ops.CmdWrite = XAie_BaremetalIO_CmdWrite,
	.Ops.RunOp = XAie_BaremetalIO_RunOp,
	.Ops.MemAllocate = XAie_BaremetalMemAllocate,
	.Ops.MemFree = XAie_BaremetalMemFree,
	.Ops.MemFreeVAddr = XAie_BaremetalMemFreeVAddr,
	.Ops.MemSyncForCPU = XAie_BaremetalMemSyncForCPU,
	.Ops.MemSyncForCPUVAddr = XAie_BaremetalMemSyncForCPUVAddr,
	.Ops.MemSyncForDev = XAie_BaremetalMemSyncForDev,
	.Ops.MemSyncForDevVAddr = XAie_BaremetalMemSyncForDevVAddr,
	.Ops.MemGetDevAddrFromVAddr = XAie_BaremetalMemGetDevAddrFromVAddr,
	.Ops.MemAttach = XAie_BaremetalMemAttach,
	.Ops.MemDetach = XAie_BaremetalMemDetach,
	.Ops.GetTid = XAie_IODummyGetTid,
	.Ops.GetPartFd = XAie_IODummyGetPartFd,
	.Ops.SubmitTxn = NULL,
	.Ops.AddressPatching = NULL,
	.Ops.SetPadInteger = NULL,
	.Ops.SetPadString = NULL,
};

/** @} */
