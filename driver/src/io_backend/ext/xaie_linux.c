/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_linux.c
* @{
*
* This file contains the low level layer IO interface for linux kernel backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus    07/29/2020  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#ifdef __AIELINUX__

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "xlnx-ai-engine.h"

#endif

#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_linux.h"
#include "xaie_npi.h"

/***************************** Macro Definitions *****************************/
#define XAIE_128BIT_ALIGN_MASK 0xFF

/****************************** Type Definitions *****************************/
#ifdef __AIELINUX__

typedef struct XAie_MemMap {
	int Fd;
	void *VAddr;
	u64 MapSize;
} XAie_MemMap;

typedef struct XAie_LinuxIO {
	int DeviceFd;		/* File descriptor of the device */
	int PartitionFd;	/* File descriptor of the partition */
	XAie_MemMap RegMap;	/* Read only mapping of registers */
	XAie_MemMap ProgMem;	/* Mapping of program memory of aie */
	XAie_MemMap DataMem;  	/* Mapping of data memory of aie */
	u64 ProgMemAddr;
	u64 ProgMemSize;
	u64 DataMemAddr;
	u64 DataMemSize;
	u32 NumMems;
	u32 NumCols;
	u32 NumRows;
	u8 RowShift;
	u8 ColShift;
	u64 BaseAddr;
} XAie_LinuxIO;

#endif /* __AIELINUX__ */

/************************** Variable Definitions *****************************/
const XAie_Backend LinuxBackend =
{
	.Type = XAIE_IO_BACKEND_LINUX,
	.Ops.Init = XAie_LinuxIO_Init,
	.Ops.Finish = XAie_LinuxIO_Finish,
	.Ops.Write32 = XAie_LinuxIO_Write32,
	.Ops.Read32 = XAie_LinuxIO_Read32,
	.Ops.MaskWrite32 = XAie_LinuxIO_MaskWrite32,
	.Ops.MaskPoll = XAie_LinuxIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_LinuxIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_LinuxIO_BlockSet32,
	.Ops.CmdWrite = XAie_LinuxIO_CmdWrite,
	.Ops.RunOp = XAie_LinuxIO_RunOp,
	.Ops.MemAllocate = XAie_LinuxMemAllocate,
	.Ops.MemFree = XAie_LinuxMemFree,
	.Ops.MemSyncForCPU = XAie_LinuxMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_LinuxMemSyncForDev,
};

/************************** Function Definitions *****************************/
#ifdef __AIELINUX__

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
* the reference count reaches a zero. Internal only.
*
*******************************************************************************/
AieRC XAie_LinuxIO_Finish(void *IOInst)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;

	munmap(LinuxIOInst->RegMap.VAddr, LinuxIOInst->RegMap.MapSize);
	munmap(LinuxIOInst->ProgMem.VAddr, LinuxIOInst->ProgMem.MapSize);
	munmap(LinuxIOInst->DataMem.VAddr, LinuxIOInst->DataMem.MapSize);

	close(LinuxIOInst->ProgMem.Fd);
	close(LinuxIOInst->DataMem.Fd);
	close(LinuxIOInst->PartitionFd);
	close(LinuxIOInst->DeviceFd);

	free(IOInst);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function requests for an available partition from kernel.
*
* @param	DevInst: Device Instance
* @param	IOInst: IO instance pointer
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. Currently, the implementation assumes that only
*		only partition is available.
*
*******************************************************************************/
static AieRC _XAie_LinuxIO_GetPartition(XAie_DevInst *DevInst,
		XAie_LinuxIO *IOInst)
{
	u32 Size;

	if(DevInst->PartProp.Handle != 0) {
		/* Requested by other modules like XRT */
		IOInst->PartitionFd = DevInst->PartProp.Handle;
	} else {
		u32 PartitionId;
		int Ret, Fd;
		struct aie_partition_query Query;
		struct aie_range_args *Partitions;
		struct aie_partition_req PartReq = {0, 0, 0, 0};

		memset(&Query, 0, sizeof(Query));
		Ret = ioctl(IOInst->DeviceFd, AIE_ENQUIRE_PART_IOCTL, &Query);
		if((Ret) < 0 || (Query.partition_cnt == 0)) {
			XAIE_ERROR("Partition query failed\n");
			return XAIE_ERR;
		}

		XAIE_DBG("%u partitions are available\n", Query.partition_cnt);
		/*
		 * TODO: Remove below check once multiple partitions are
		 *	 supported on AIE. Since only one partition is
		 *	 supported currently, any partition id returned by the
		 *	 kernel is acceptable. Additional checks have to be
		 *	 performed based on what partition id is passed by the
		 *	 user.
		 */
		if(Query.partition_cnt > 1) {
			XAIE_ERROR("Multiple partitions are not supported"
					"currently\n");
			return XAIE_FEATURE_NOT_SUPPORTED;
		}

		Partitions = (struct aie_range_args *)
			malloc(Query.partition_cnt * sizeof(*Partitions));
		if(Partitions == NULL) {
			XAIE_ERROR("Failed to alloacte memory for partition"
					"requests\n");
			return XAIE_ERR;
		}

		Query.partitions = Partitions;
		Ret = ioctl(IOInst->DeviceFd, AIE_ENQUIRE_PART_IOCTL, &Query);
		if(Ret < 0) {
			free(Partitions);
			XAIE_ERROR("Failed to enquire paritions\n");
			return XAIE_ERR;
		}

		/* Since there is only one partition today pick the first one */
		PartitionId = Query.partitions[0].partition_id;

		/* Setup partition request arguments */
		PartReq.partition_id = PartitionId;
		PartReq.flag = DevInst->PartProp.CntrFlag;
		Fd = ioctl(IOInst->DeviceFd, AIE_REQUEST_PART_IOCTL, &PartReq);
		if(Fd < 0) {
			XAIE_ERROR("Failed to request partition %u.\n",
					PartitionId);
			free(Partitions);
			return XAIE_ERR;
		}

		free(Partitions);
		IOInst->PartitionFd = Fd;
		XAIE_DBG("Partition request successful. Partition id is %u\n",
				PartitionId);
	}

	IOInst->NumCols = DevInst->NumCols;
	IOInst->NumRows = DevInst->NumRows;
	Size = IOInst->NumCols << IOInst->ColShift;
	/* mmap register space as read only */
	IOInst->RegMap.VAddr = mmap(NULL, Size, PROT_READ, MAP_SHARED,
			IOInst->PartitionFd, 0);
	if(IOInst->RegMap.VAddr == MAP_FAILED) {
		XAIE_ERROR("Failed to map register space for read"
				" operations\n");
		return XAIE_ERR;
	}

	IOInst->RegMap.MapSize = Size;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function memory maps the program and data memories of individual aie
* tiles for read/write operations.
*
* @param	DevInst: Device Instance
* @param	IOInst: IO instance pointer
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_LinuxIO_MapMemory(XAie_DevInst *DevInst,
		XAie_LinuxIO *IOInst)
{
	int Ret;
	struct aie_mem_args MemArgs = {0, NULL};
	const XAie_CoreMod *CoreMod =
		DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;
	const XAie_MemMod *MemMod =
		DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].MemMod;

	Ret = ioctl(IOInst->PartitionFd, AIE_GET_MEM_IOCTL, &MemArgs);
	if(Ret < 0) {
		XAIE_ERROR("Failed to get number of memories\n");
		return XAIE_ERR;
	}

	if(MemArgs.num_mems > 2) {
		XAIE_ERROR("Got invalid number of memories: %u\n",
				MemArgs.num_mems);
		return XAIE_ERR;
	}

	MemArgs.mems = (struct aie_mem *)malloc(sizeof(*MemArgs.mems) *
			MemArgs.num_mems);
	if(MemArgs.mems == NULL) {
		XAIE_ERROR("Memory allocation failed\n");
		return XAIE_ERR;
	}

	Ret = ioctl(IOInst->PartitionFd, AIE_GET_MEM_IOCTL, &MemArgs);
	if(Ret < 0) {
		XAIE_ERROR("Failed to get memory information.\n");
		free(MemArgs.mems);
		return XAIE_ERR;
	}

	for(u32 i = 0U; i < MemArgs.num_mems; i++) {
		void *MemVAddr;
		struct aie_mem *Mem = &MemArgs.mems[i];
		u64 MMapSize = Mem->size * Mem->range.size.col *
			Mem->range.size.row;

		MemVAddr = mmap(NULL, MMapSize, PROT_READ | PROT_WRITE,
				MAP_SHARED, Mem->fd, 0);
		if(MemVAddr == MAP_FAILED) {
			XAIE_ERROR("Failed to mmap memory. Offset 0x%x\n",
					Mem->offset);
			free(MemArgs.mems);
			return XAIE_ERR;
		}

		if(Mem->offset == CoreMod->ProgMemHostOffset) {
			IOInst->ProgMem.VAddr = MemVAddr;
			IOInst->ProgMem.MapSize = MMapSize;
			IOInst->ProgMem.Fd = Mem->fd;
		} else if(Mem->offset == MemMod->MemAddr) {
			IOInst->DataMem.VAddr = MemVAddr;
			IOInst->DataMem.MapSize = MMapSize;
			IOInst->DataMem.Fd = Mem->fd;
		} else {
			XAIE_ERROR("Memory offset 0x%x is not valid.",
					Mem->offset);
			free(MemArgs.mems);
			return XAIE_ERR;
		}
	}

	XAIE_DBG("Prog memory mapped to 0x%p\n", IOInst->ProgMem.VAddr);
	XAIE_DBG("Data memory mapped to 0x%p\n", IOInst->DataMem.VAddr);

	IOInst->ProgMemAddr = CoreMod->ProgMemHostOffset;
	IOInst->ProgMemSize = CoreMod->ProgMemSize;
	IOInst->DataMemAddr = MemMod->MemAddr;
	IOInst->DataMemSize = MemMod->Size;

	free(MemArgs.mems);

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
* to initialize just increments the reference count. Internal only.
*
*******************************************************************************/
AieRC XAie_LinuxIO_Init(XAie_DevInst *DevInst)
{
	AieRC RC;
	XAie_LinuxIO *IOInst;
	int Fd;

	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
		XAIE_ERROR("Backend not supported for device\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	IOInst = (XAie_LinuxIO *)malloc(sizeof(*IOInst));
	if(IOInst == NULL) {
		XAIE_ERROR("Initialization failed. Failed to allocate memory\n");
		return XAIE_ERR;
	}

	Fd = open("/dev/aie0", O_RDWR);
	if(Fd < 0) {
		XAIE_ERROR("Failed to open aie device %s\n", "/dev/aie0");
		free(IOInst);
		return XAIE_ERR;
	}

	IOInst->RowShift = DevInst->DevProp.RowShift;
	IOInst->ColShift = DevInst->DevProp.ColShift;
	IOInst->BaseAddr = DevInst->BaseAddr;
	IOInst->DeviceFd = Fd;

	RC = _XAie_LinuxIO_GetPartition(DevInst, IOInst);
	if(RC != XAIE_OK) {
		free(IOInst);
		return RC;
	}

	XAIE_DBG("Registers mapped as read-only to 0x%lx\n",
			IOInst->RegMap.VAddr);

	RC = _XAie_LinuxIO_MapMemory(DevInst, IOInst);
	if(RC != XAIE_OK) {
		free(IOInst);
		return XAIE_ERR;
	}

	DevInst->IOInst = (void *)IOInst;

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
void XAie_LinuxIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	int Ret;
	struct aie_reg_args Args;

	Args.op = AIE_REG_WRITE;
	Args.offset = RegOff;
	Args.val = Value;
	Args.mask = 0; /* mask must be 0 for register write */

	/*
	 * TBD: Is the check of ioctl call required here? Other backends do not
	 * check for errors. Kernels prints error messages anyway.
	 */
	Ret = ioctl(LinuxIOInst->PartitionFd, AIE_REG_IOCTL, &Args);
	if(Ret < 0) {
		XAIE_ERROR("Register write failed for offset 0x%lx.\n", RegOff);
	}
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
* @note		Internal only.
*
*******************************************************************************/
u32 XAie_LinuxIO_Read32(void *IOInst, u64 RegOff)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;

	return *((u32 *)(LinuxIOInst->RegMap.VAddr + RegOff));
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
* @note		Internal only.
*
*******************************************************************************/
void XAie_LinuxIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Value)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	int Ret;
	struct aie_reg_args Args;

	Args.op = AIE_REG_WRITE;
	Args.offset = RegOff;
	Args.val = Value;
	Args.mask = Mask;

	/*
	 * TBD: Is the check of ioctl call required here? Other backends do not
	 * check for errors. Kernels prints error messages anyway.
	 */
	Ret = ioctl(LinuxIOInst->PartitionFd, AIE_REG_IOCTL, &Args);
	if(Ret < 0) {
		XAIE_ERROR("Register write failed for offset 0x%lx.\n", RegOff);
	}
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
* @note		Internal only.
*
*******************************************************************************/
u32 XAie_LinuxIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
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
		if((XAie_LinuxIO_Read32(IOInst, RegOff) & Mask) == Value) {
			Ret = XAIELIB_SUCCESS;
			break;
		}
		XAieLib_usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	if((Ret == XAIELIB_FAILURE) &&
			((XAie_LinuxIO_Read32(IOInst, RegOff) & Mask) ==
			 Value)) {
		Ret = XAIELIB_SUCCESS;
	}

	return Ret;
}

/*****************************************************************************/
/**
*
* This function returns the register address of individual tiles for a given
* offset that includes row and columns offsets.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
*
* @return	Register address.
*
* @note		Internal only.
*
*******************************************************************************/
static inline u64 _XAie_GetRegAddr(XAie_LinuxIO *IOInst, u64 RegOff)
{
	return RegOff & (~(ULONG_MAX << IOInst->RowShift));
}

/*****************************************************************************/
/**
*
* This function returns the row number for a given register offset.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
*
* @return	Column number.
*
* @note		Internal only.
*
*******************************************************************************/
static inline u8 _XAie_GetRowNum(XAie_LinuxIO *IOInst, u64 RegOff)
{
	u64 Mask = (IOInst->ColShift - IOInst->RowShift) << IOInst->RowShift;

	return (RegOff & Mask) >> IOInst->RowShift;
}

/*****************************************************************************/
/**
*
* This function returns the column number for a given register offset.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
*
* @return	Column number.
*
* @note		Internal only.
*
*******************************************************************************/
static inline u8 _XAie_GetColNum(XAie_LinuxIO *IOInst, u64 RegOff)
{
	return RegOff >> IOInst->ColShift;
}

/*****************************************************************************/
/**
*
* This function computes the offset from the memory mapped region for a given
* Col, Row coordinate of the aie tile.
*
* @param	IOInst: IO instance pointer
* @param	Col: Column number
* @param	Row: Row number
* @param	MemSize: Total size of the PM/DM for the aie tile.
*
* @return	Memory offset.
*
* @note		Internal only. This api currently works for aie only. With the
*		assumption that row 0 is shim row.
*
*******************************************************************************/
static inline u64 _XAie_GetMemOffset(XAie_LinuxIO *IOInst, u8 Col, u8 Row,
		u64 MemSize)
{
	/*
	 * 1 has to be subtracted from the num rows to account for the
	 * shim row.
	 */
	return ((u64)(Col * (IOInst->NumRows - 1)) * MemSize) +
		((u64)(Row - 1) * MemSize);
}

/*****************************************************************************/
/**
*
* This function copies data to the device using memcpy. The function accepts
* a 32 bit aligned address. 128bit unaligned addresses are managed within the
* funtion.
*
* @param	Dest: Pointer to the destination address.
* @param	Src: Pointer to the source buffer.
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_CopyDataToMem(u32 *Dest, u32 *Src, u32 Size)
{
	u32 StartExtraWrds = 0, EndExtraWrds = 0;

	if((u64)Dest & XAIE_128BIT_ALIGN_MASK) {
		StartExtraWrds = ((XAIE_128BIT_ALIGN_MASK + 1) -
				(((u64)Dest) & XAIE_128BIT_ALIGN_MASK)) / 4;
	}

	if(StartExtraWrds > 0) {
		for(u8 i = 0; i < StartExtraWrds && Size > 0;  i++) {
			*Dest = *Src;
			Dest++;
			Src++;
			Size--;
		}
	}

	if((u64)(Dest + Size) & XAIE_128BIT_ALIGN_MASK) {
		EndExtraWrds = (((u64)(Dest + Size)) &
				XAIE_128BIT_ALIGN_MASK) / 4;
	}

	if(Size >= (XAIE_128BIT_ALIGN_MASK + 1)) {
		memcpy((void *)Dest, (void *)Src,
				(Size - EndExtraWrds) * sizeof(u32));
		Dest += Size - EndExtraWrds;
		Src += Size - EndExtraWrds;
		Size -= Size - EndExtraWrds;
	}

	if(Size > 0) {
		for(u8 i = 0; Size > 0; i++) {
			*Dest = *Src;
			Dest++;
			Src++;
			Size--;
		}
	}

	if(Size != 0) {
		XAIE_ERROR("Copying data to memory failed. %u words not"
				"written to device\n", Size);
	}
}

/*****************************************************************************/
/**
*
* This function returns the virtual address of the memory mapped PM/DM section
* of the array given a register offset.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Size: Number of 32-bit words.
*
* @return	Valid address on success. NULL on non PM/DM register offset.
*
* @note		Internal only.
*
*******************************************************************************/
static u32* _XAie_GetVirtAddrFromOffset(XAie_LinuxIO *IOInst, u64 RegOff,
		u32 Size)
{
	u32 *VirtAddr = NULL;
	u64 MemOffset;
	u64 RegAddr = _XAie_GetRegAddr(IOInst, RegOff);
	u8 Row = _XAie_GetRowNum(IOInst, RegOff);
	u8 Col = _XAie_GetColNum(IOInst, RegOff);

	if(((RegAddr + Size) < (IOInst->ProgMemAddr + IOInst->ProgMemSize)) &&
			(RegAddr >= IOInst->ProgMemAddr)) {
		/* Handle program memory block write */
		MemOffset = _XAie_GetMemOffset(IOInst, Col, Row,
				IOInst->ProgMemSize);
		VirtAddr = (u32 *)((char *) IOInst->ProgMem.VAddr + MemOffset +
				RegAddr - IOInst->ProgMemAddr);
	} else if(((RegAddr + Size) < (IOInst->DataMemAddr +
					IOInst->DataMemSize)) &&
			(RegAddr >= IOInst->DataMemAddr)) {
		/* Handle data memory block write */
		MemOffset = _XAie_GetMemOffset(IOInst, Col, Row,
				IOInst->DataMemSize);
		VirtAddr = (u32 *)((char *)IOInst->DataMem.VAddr + MemOffset +
				RegAddr);
	}

	return VirtAddr;
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
void XAie_LinuxIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	XAie_LinuxIO *Inst = (XAie_LinuxIO *)IOInst;
	u32 *VirtAddr;

	/* Handle PM and DM sections */
	VirtAddr =  _XAie_GetVirtAddrFromOffset(Inst, RegOff, Size);
	if(VirtAddr != NULL) {
		_XAie_CopyDataToMem(VirtAddr, Data, Size);
		return;
	}

	/* Handle other registers */
	for(u32 i = 0; i < Size; i++) {
		XAie_LinuxIO_Write32(IOInst, RegOff + i * 4U, *Data);
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
* @note		Internal only.
*
*******************************************************************************/
void XAie_LinuxIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	XAie_LinuxIO *Inst = (XAie_LinuxIO *)IOInst;
	u32 *VirtAddr;

	/* Handle PM and DM sections */
	VirtAddr =  _XAie_GetVirtAddrFromOffset(Inst, RegOff, Size);
	if(VirtAddr != NULL) {
		for(u32 i = 0; i < Size; i++) {
			*VirtAddr++ = Data;
		}
		return;
	}

	/* Handle other registers */
	for(u32 i = 0; i < Size; i++) {
		XAie_LinuxIO_Write32(IOInst, RegOff + i * 4U, Data);
	}
}

#else

AieRC XAie_LinuxIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

AieRC XAie_LinuxIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with Linux kernel backend "
			"(__AIELINUX__)\n");
	return XAIE_INVALID_BACKEND;
}

u32 XAie_LinuxIO_Read32(void *IOInst, u64 RegOff)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	return 0;
}

void XAie_LinuxIO_Write32(void *IOInst, u64 RegOff, u32 Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
}

void XAie_LinuxIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask, u32 Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Data;
}

u32 XAie_LinuxIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;
	return XAIELIB_FAILURE;
}

void XAie_LinuxIO_BlockWrite32(void *IOInst, u64 RegOff, u32 *Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
}

void XAie_LinuxIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
}

#endif /* __AIEMETAL__ */

void XAie_LinuxIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command, u32 CmdWd0,
		u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
	(void)IOInst;
	(void)Col;
	(void)Row;
	(void)Command;
	(void)CmdWd0;
	(void)CmdWd1;
	(void)CmdStr;
}

XAie_MemInst* XAie_LinuxMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

AieRC XAie_LinuxMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_LinuxMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_LinuxMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

AieRC XAie_LinuxIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

/** @} */
