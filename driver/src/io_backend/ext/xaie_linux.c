/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
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
#include <linux/dma-buf.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <liburing.h>

#include "xlnx-ai-engine.h"

#endif

#include "xaie_helper.h"
#include "xaie_helper_internal.h"
#include "xaie_io.h"
#include "xaie_io_common.h"
#include "xaie_npi.h"
#include "xaie_perfcnt.h"
#include "xaie_reset_aie.h"

/***************************** Macro Definitions *****************************/
#define XAIE_128BIT_ALIGN_MASK 0xF
#define XAIE_DEVICE_FILE "/dev/aie0"
#define XAIE_OCCUPANCY_USER_EVENT_NUM 0x2U
#define AIE_IO_URING_BUFFER_ENTRIES 32U

#ifdef __AIELINUX__

/***************************** Global Variable *******************************/
static XAie_PerfInst *UserInst = NULL;
static void *IOInstLinux = NULL;
static timer_t TimerID;

/****************************** Type Definitions *****************************/

typedef struct XAie_MemMap {
	int Fd;
	void *VAddr;
	u64 MapSize;
} XAie_MemMap;

typedef struct XAie_LinuxIO {
	XAie_DevInst *DevInst;
	int DeviceFd;		/* File descriptor of the device */
	int PartitionFd;	/* File descriptor of the partition */
	XAie_MemMap RegMap;	/* Read only mapping of registers */
	XAie_MemMap ProgMem;	/* Mapping of program memory of aie */
	XAie_MemMap DataMem;  	/* Mapping of data memory of aie */
	XAie_MemMap MemTileMem;	/* Mapping of memory tile mem */
	XAie_MemMap UcProgMem;	/* Mapping of UC Program memory */
	XAie_MemMap UcPrivDataMem; /* Mapping of UC Data memory */
	XAie_MemMap UcDataMem;	/* Mapping of shared UC Data memory */
	u64 ProgMemAddr;
	u64 ProgMemSize;
	u64 DataMemAddr;
	u64 DataMemSize;
	u64 MemTileMemAddr;
	u64 MemTileMemSize;
	u32 NumMems;
	u32 NumCols;
	u32 NumRows;
	u8 RowShift;
	u8 ColShift;
	u64 BaseAddr;
	struct io_uring Ring;
	struct io_uring_params Params;
	struct iovec *IoVecs;
	u32 IoVecsCount;
	u32 IoVecsHead;
	u32 IoVecsTail;

} XAie_LinuxIO;

typedef struct XAie_LinuxMem {
	int BufferFd;
} XAie_LinuxMem;

/************************** Function Definitions *****************************/

static bool _XAie_LinuxIO_IoVecIsFull(XAie_LinuxIO *LinuxIOInst)
{
	return LinuxIOInst->IoVecsHead == LinuxIOInst->IoVecsTail;
}
static u32 _XAie_LinuxIO_IoVecIncHead(XAie_LinuxIO *LinuxIOInst)
{
	u32 ret = LinuxIOInst->IoVecsHead;

	LinuxIOInst->IoVecsHead = (LinuxIOInst->IoVecsHead + 1) % LinuxIOInst->IoVecsCount;
	return ret;
}

static u32 _XAie_LinuxIO_IoVecIncTail(XAie_LinuxIO *LinuxIOInst)
{
	u32 ret = LinuxIOInst->IoVecsTail;

	LinuxIOInst->IoVecsTail = (LinuxIOInst->IoVecsTail + 1) % LinuxIOInst->IoVecsCount;
	return ret;
}

static u32 _XAie_LinuxIO_IoVecDecHead(XAie_LinuxIO *LinuxIOInst)
{
	u32 ret = LinuxIOInst->IoVecsHead;

	LinuxIOInst->IoVecsHead = (LinuxIOInst->IoVecsHead - 1 + LinuxIOInst->IoVecsCount) % LinuxIOInst->IoVecsCount;
	return ret;
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
	u64 Mask = ((1 << IOInst->ColShift) - 1) &
			~((1 << IOInst->RowShift) - 1);

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
static AieRC XAie_LinuxIO_Finish(void *IOInst)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	const XAie_MemMod *MemTileMod =
		LinuxIOInst->DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_MEMTILE].MemMod;
	const XAie_UcMod *UcMod =
		LinuxIOInst->DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].UcMod;

	munmap(LinuxIOInst->RegMap.VAddr, LinuxIOInst->RegMap.MapSize);
	if(MemTileMod != NULL)
		munmap(LinuxIOInst->MemTileMem.VAddr, LinuxIOInst->MemTileMem.MapSize);

	munmap(LinuxIOInst->ProgMem.VAddr, LinuxIOInst->ProgMem.MapSize);
	munmap(LinuxIOInst->DataMem.VAddr, LinuxIOInst->DataMem.MapSize);
	if (UcMod) {
		munmap(LinuxIOInst->UcProgMem.VAddr, LinuxIOInst->UcProgMem.MapSize);
		munmap(LinuxIOInst->UcPrivDataMem.VAddr, LinuxIOInst->UcPrivDataMem.MapSize);
		munmap(LinuxIOInst->UcDataMem.VAddr, LinuxIOInst->UcDataMem.MapSize);
	}

	close(LinuxIOInst->ProgMem.Fd);
	close(LinuxIOInst->DataMem.Fd);
	if(MemTileMod != NULL)
		close(LinuxIOInst->MemTileMem.Fd);
	if (UcMod) {
		close(LinuxIOInst->UcProgMem.Fd);
		close(LinuxIOInst->UcPrivDataMem.Fd);
		close(LinuxIOInst->UcDataMem.Fd);
	}

	io_uring_unregister_files(&LinuxIOInst->Ring);
	io_uring_unregister_buffers(&LinuxIOInst->Ring);
	for (u32 i = 0; i < LinuxIOInst->IoVecsCount; i++) {
		if (LinuxIOInst->IoVecs[i].iov_base != NULL) {
			munmap(LinuxIOInst->IoVecs[i].iov_base, LinuxIOInst->IoVecs[i].iov_len);
		}
	}
	io_uring_queue_exit(&LinuxIOInst->Ring);
	free(LinuxIOInst->IoVecs);

	close(LinuxIOInst->PartitionFd);
	close(LinuxIOInst->DeviceFd);

	free(IOInst);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function will get the partition details from kernel and will appends the
* partition details to list.
*
* @param	DevInst: Global AIE device instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
static AieRC XAie_LinuxIO_GetPartitionList(XAie_DevInst *DevInst)
{
	int Ret;
	int AieDevFd, cnt = 0;
	struct aie_part_fd_list aiepart_list;
	struct aie_partition_query PartQuery;

	AieDevFd = open(XAIE_DEVICE_FILE, O_RDWR);
	if (AieDevFd < 0) {
		XAIE_ERROR("Failed to open aie device in GetPartitionFd %s, \
				%d: %s\n","/dev/aie0", errno, strerror(errno));
		return XAIE_ERR;
	}

	PartQuery.partitions = NULL;
	Ret = ioctl(AieDevFd, AIE_ENQUIRE_PART_IOCTL, &PartQuery);
	if (Ret < 0) {
		XAIE_ERROR("Failed to get partitions count %d: %s\n",
				errno, strerror(errno));
		return XAIE_ERR;
	}

	aiepart_list.num_entries = PartQuery.partition_cnt;
	if (aiepart_list.num_entries == 0) {
		XAIE_ERROR("Partitions was not created, create Partition first\n");
		return XAIE_ERR;
	}

	aiepart_list.list = (struct aie_part_fd *) malloc(
			aiepart_list.num_entries * sizeof(struct aie_part_fd));
	if (aiepart_list.list == NULL) {
		XAIE_ERROR("failed to allocate memory for partition list\n");
		return XAIE_ERR;
	}

	Ret = ioctl(AieDevFd, AIE_GET_PARTITION_FD_LIST_IOCTL, &aiepart_list);
	if (Ret < 0) {
		XAIE_ERROR("Failed to get partition list %d: %s\n",
				errno, strerror(errno));
		return XAIE_ERR;
	}

	for (cnt = 0; cnt < aiepart_list.num_entries; cnt++) {
		XAie_PartitionList *PartInst = (XAie_PartitionList *) malloc(
				sizeof(XAie_PartitionList));
		if (PartInst == NULL) {
			XAIE_ERROR("failed to allocate memory for partition list\n");
			return XAIE_ERR;

		}

		PartInst->ColRange.Start =
			aiepart_list.list[cnt].col_args.start_col;
		PartInst->ColRange.Num =
			aiepart_list.list[cnt].col_args.num_cols;
		PartInst->PartitionId = aiepart_list.list[cnt].partition_id;
		PartInst->Uid = aiepart_list.list[cnt].uid;
		PartInst->PartitionFd = aiepart_list.list[cnt].fd;

		_XAie_AppendPartitionToList(DevInst, PartInst);
	}

	close(AieDevFd);
	free(aiepart_list.list);

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
		struct aie_partition_req PartReq = {0};
		u32 PartitionId;
		int Fd;

		/* Since there is only one partition today pick the first one */
		PartitionId = DevInst->StartCol << AIE_PART_ID_START_COL_SHIFT;
		PartitionId += DevInst->NumCols << AIE_PART_ID_NUM_COLS_SHIFT;

		/* Setup partition request arguments */
		PartReq.partition_id = PartitionId;
		PartReq.flag = DevInst->PartProp.CntrFlag;
		Fd = ioctl(IOInst->DeviceFd, AIE_REQUEST_PART_IOCTL, &PartReq);
		if(Fd < 0) {
			XAIE_ERROR("Failed to request partition %u, %d: %s\n",
					PartitionId, errno, strerror(errno));
			return XAIE_ERR;
		}

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
				" operations, %d: %s\n",
				errno, strerror(errno));
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
	const XAie_MemMod *MemTileMod =
		DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_MEMTILE].MemMod;
	const XAie_UcMod *UcMod =
		DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].UcMod;

	Ret = ioctl(IOInst->PartitionFd, AIE_GET_MEM_IOCTL, &MemArgs);
	if(Ret < 0) {
		XAIE_ERROR("Failed to get number of memories, %d: %s\n",
			errno, strerror(errno));
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
		XAIE_ERROR("Failed to get memory information, %d: %s\n",
			errno, strerror(errno));
		free(MemArgs.mems);
		return XAIE_ERR;
	}

	for(u32 i = 0U; i < MemArgs.num_mems; i++) {
		void *MemVAddr;
		struct aie_mem *Mem = &MemArgs.mems[i];
		u64 MMapSize = Mem->size * Mem->range.size.col *
			Mem->range.size.row;

		XAIE_DBG("Mapping memory: Offset: 0x%x, Size: 0x%x, MMapSize: 0x%lx\n",
				Mem->offset, Mem->size, MMapSize);
		MemVAddr = mmap(NULL, MMapSize, PROT_READ | PROT_WRITE,
				MAP_SHARED, Mem->fd, 0);
		if(MemVAddr == MAP_FAILED) {
			XAIE_ERROR("Failed to mmap memory. Offset 0x%x, %d: %s\n",
					Mem->offset, errno, strerror(errno));
			free(MemArgs.mems);
			return XAIE_ERR;
		}

		if(Mem->offset == CoreMod->ProgMemHostOffset) {
			IOInst->ProgMem.VAddr = MemVAddr;
			IOInst->ProgMem.MapSize = MMapSize;
			IOInst->ProgMem.Fd = Mem->fd;
		} else if((Mem->offset == MemMod->MemAddr) &&
				(Mem->size == MemMod->Size)) {
			IOInst->DataMem.VAddr = MemVAddr;
			IOInst->DataMem.MapSize = MMapSize;
			IOInst->DataMem.Fd = Mem->fd;
		} else if((MemTileMod != NULL) &&
			  (Mem->offset == MemTileMod->MemAddr) &&
			  (Mem->size == MemTileMod->Size)) {
			IOInst->MemTileMem.VAddr = MemVAddr;
			IOInst->MemTileMem.MapSize = MMapSize;
			IOInst->MemTileMem.Fd = Mem->fd;
		} else if (UcMod &&
			   (Mem->offset == UcMod->ProgMemHostOffset)) {
			IOInst->UcProgMem.VAddr = MemVAddr;
			IOInst->UcProgMem.MapSize = MMapSize;
			IOInst->UcProgMem.Fd = Mem->fd;
		} else if (UcMod &&
			   (Mem->offset == UcMod->PrivDataMemAddr)) {
			IOInst->UcPrivDataMem.VAddr = MemVAddr;
			IOInst->UcPrivDataMem.MapSize = MMapSize;
			IOInst->UcPrivDataMem.Fd = Mem->fd;
		} else if (UcMod &&
			   (Mem->offset == UcMod->DataMemAddr)) {
			IOInst->UcDataMem.VAddr = MemVAddr;
			IOInst->UcDataMem.MapSize = MMapSize;
			IOInst->UcDataMem.Fd = Mem->fd;

		} else {
			XAIE_ERROR("Memory offset 0x%x is not valid.\n",
					Mem->offset);
			free(MemArgs.mems);
			return XAIE_ERR;
		}
	}

	XAIE_DBG("Prog memory mapped to %p\n", IOInst->ProgMem.VAddr);
	XAIE_DBG("Data memory mapped to %p\n", IOInst->DataMem.VAddr);
	XAIE_DBG("Mem tile memory mapped to %p\n", IOInst->MemTileMem.VAddr);
	if (UcMod) {
		XAIE_DBG("UcProgMem mapped to %p\n", IOInst->UcProgMem.VAddr);
		XAIE_DBG("UcPrivDataMem mapped to %p\n", IOInst->UcPrivDataMem.VAddr);
		XAIE_DBG("UcDataMem mapped to %p\n", IOInst->UcDataMem.VAddr);
	}

	IOInst->ProgMemAddr = CoreMod->ProgMemHostOffset;
	IOInst->ProgMemSize = CoreMod->ProgMemSize;
	IOInst->DataMemAddr = MemMod->MemAddr;
	IOInst->DataMemSize = MemMod->Size;

	if (MemTileMod != NULL) {
		IOInst->MemTileMemAddr = MemTileMod->MemAddr;
		IOInst->MemTileMemSize = MemTileMod->Size;
	} else {
		IOInst->MemTileMemAddr = 0L;
		IOInst->MemTileMemSize = 0L;
	}

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
static AieRC XAie_LinuxIO_Init(XAie_DevInst *DevInst)
{
	AieRC RC;
	XAie_LinuxIO *IOInst;
	int ret;
	int Fd;
	u32 NumTiles;
	u32 SetTileStatus;
	u16 RingSize = 1024;
	size_t PgSize = getpagesize();

	IOInst = (XAie_LinuxIO *)calloc(1, sizeof(*IOInst));
	if(IOInst == NULL) {
		XAIE_ERROR("Initialization failed. Failed to allocate memory\n");
		return XAIE_ERR;
	}

	Fd = open(XAIE_DEVICE_FILE, O_RDWR);
	if(Fd < 0) {
		XAIE_ERROR("Failed to open aie device %s, %d: %s\n",
			"/dev/aie0", errno, strerror(errno));
		RC = XAIE_ERR;
		goto free_IOInst;
	}

	IOInst->Params.flags = IORING_SETUP_CQE32 | IORING_SETUP_SQE128 |
			       IORING_SETUP_SQPOLL;

	IOInst->Params.sq_thread_idle = 1;
	if (DevInst->RingSize)
		RingSize = DevInst->RingSize;
	DevInst->RingSize = RingSize;
	ret = io_uring_queue_init_params(RingSize, &IOInst->Ring, &IOInst->Params);
	if (ret) {
		XAIE_ERROR("Uring init failed: %d\n", ret);
		RC = XAIE_ERR;
		goto free_IOInst;
	}

	IOInst->RowShift = DevInst->DevProp.RowShift;
	IOInst->ColShift = DevInst->DevProp.ColShift;
	IOInst->BaseAddr = DevInst->BaseAddr;
	IOInst->DeviceFd = Fd;

	RC = _XAie_LinuxIO_GetPartition(DevInst, IOInst);
	if(RC != XAIE_OK) {
		goto queue_exit;
	}

	XAIE_DBG("Registers mapped as read-only to 0x%lx\n",
			IOInst->RegMap.VAddr);

	RC = _XAie_LinuxIO_MapMemory(DevInst, IOInst);
	if(RC != XAIE_OK) {
		goto queue_exit;
	}

	XAie_LocType TileLoc = XAie_TileLoc(0, 1);
	NumTiles = (DevInst->NumRows - 1U) * (DevInst->NumCols);
	SetTileStatus = _XAie_GetTileBitPosFromLoc(DevInst, TileLoc);
	_XAie_SetBitInBitmap(DevInst->DevOps->TilesInUse, SetTileStatus,
				NumTiles);

	DevInst->IOInst = (void *)IOInst;
	IOInst->DevInst = DevInst;

	ret = io_uring_register_files(&IOInst->Ring, &IOInst->PartitionFd, 1);
	if (ret) {
		XAIE_ERROR("Failed to register fd for uring sqpoll: %d\n", ret);
		RC = XAIE_ERR;
		goto queue_exit;
	}
	IOInst->IoVecsCount = AIE_IO_URING_BUFFER_ENTRIES;
	IOInst->IoVecs = (struct iovec *)calloc(IOInst->IoVecsCount, sizeof(struct iovec));
	if (IOInst->IoVecs == NULL) {
		XAIE_ERROR("Failed to allocate memory for iovec structures\n");
		RC = XAIE_ERR;
		goto queue_exit;
	}
	for (u32 i = 0; i < IOInst->IoVecsCount; i++) {
		IOInst->IoVecs[i].iov_base = mmap(NULL, PgSize, PROT_READ | PROT_WRITE,
						  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		if (IOInst->IoVecs[i].iov_base == MAP_FAILED) {
			XAIE_ERROR("Failed to allocate memory for iovec buffer %d\n", i);
			IOInst->IoVecs[i].iov_base = NULL;
			RC = XAIE_ERR;
			goto free_iovecs;
		}
		IOInst->IoVecs[i].iov_len = PgSize;
	}
	ret = io_uring_register_buffers(&IOInst->Ring, IOInst->IoVecs, IOInst->IoVecsCount);
	if (ret) {
		XAIE_ERROR("Failed to register buffers for uring: %d\n", ret);
		RC = XAIE_ERR;
		goto free_iovecs;
	}
	IOInst->IoVecsHead = 0;
	IOInst->IoVecsTail = IOInst->IoVecsCount - 1;

	return XAIE_OK;

free_iovecs:
	for (u32 i = 0; i < IOInst->IoVecsCount; i++) {
		if (IOInst->IoVecs[i].iov_base != NULL) {
			munmap(IOInst->IoVecs[i].iov_base, PgSize);
		}
	}
queue_exit:
	io_uring_queue_exit(&IOInst->Ring);
free_IOInst:
	free(IOInst);
	return RC;

}

/*****************************************************************************/
/**
*
* This is the IO function to asynchronously write 32-bit data to the specified
* register address using io_uring. The register write arguments are embedded
* directly in the SQE command buffer.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to write to.
* @param	Value: 32-bit data to be written.
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking. On error, AsyncRes->res is set to the
*			corresponding error code.
*
* @return	Number of SQEs submitted on success, or 0 on failure.
*
* @note		Internal only. Uses io_uring for async submission.
*
*******************************************************************************/
static int XAie_LinuxIO_Write32_Async(void *IOInst, u64 RegOff, u32 Value,
					XAie_AsyncRes *AsyncRes)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	struct aie_reg_args *Args;
	struct io_uring_sqe *Sqe;
	int Ret;

	AsyncRes->io_vec_inuse = 0;
	Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
	if (!Sqe) {
		XAIE_ERROR("Failed to get sqe for async write\n");
		AsyncRes->res = -ENOMEM;
		return 0;
	}
	Sqe->opcode = IORING_OP_URING_CMD;
	Sqe->flags |= IOSQE_FIXED_FILE;
	Sqe->cmd_op = AIE_REG_WRITE_CMD;
	Sqe->user_data = (u64)AsyncRes;
	Args = (struct aie_reg_args *)Sqe->cmd;
	*Args = (struct aie_reg_args){
		.op = AIE_REG_WRITE,
		.offset = RegOff,
		.val = Value,
		.mask = 0, /* mask must be 0 for register write */
	};

	Ret = io_uring_submit(&LinuxIOInst->Ring);
	if (Ret < 0) {
		XAIE_ERROR("Failed to submit async write: %d\n", Ret);
		AsyncRes->res = Ret;
		return 0;
	}
	return 1;
}

/*****************************************************************************/
/**
*
* This function waits for a specified number of asynchronous operations to
* complete using io_uring.
*
* @param	IOInst: IO instance pointer
* @param	Nr: Number of completions to wait for.
*
* @return	XAIE_OK on success, XAIE_ERR on failure.
*
* @note		Internal only. Populates AsyncRes structures with completion
*		results from the completion queue entries.
*
*******************************************************************************/
static AieRC XAie_LinuxIO_AsyncWaitNr(void *IOInst, u32 Nr)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	XAie_DevInst *DevInst = LinuxIOInst->DevInst;
	struct io_uring_cqe *Cqe;
	XAie_AsyncRes *AsyncRes;
	int Ret;
	int head;

	if (Nr > DevInst->RingSize) {
		XAIE_ERROR("Requested completion wait number %u is greater than ring size %u\n",
			   Nr, DevInst->RingSize);
		return XAIE_ERR;
	}
	if (Nr == 0) {
		return XAIE_OK;
	}
	Ret = io_uring_wait_cqes(&LinuxIOInst->Ring, &Cqe, Nr, NULL, NULL);
	if (Ret < 0) {
		XAIE_ERROR("Failed to wait for async completion: %d\n", Ret);
		return XAIE_ERR;
	}
	io_uring_for_each_cqe(&LinuxIOInst->Ring, head, Cqe) {
		AsyncRes = (XAie_AsyncRes *)(uintptr_t)Cqe->user_data;
		if (Cqe->res < 0) {
			XAIE_ERROR("Async operation failed: %d\n", Cqe->res);
		}
		AsyncRes->res = Cqe->res;
		AsyncRes->res2 = Cqe->big_cqe[0];
		AsyncRes->res3 = Cqe->big_cqe[1];
		if (AsyncRes->io_vec_inuse == 1) {
			_XAie_LinuxIO_IoVecIncTail(LinuxIOInst);
		}
		io_uring_cqe_seen(&LinuxIOInst->Ring, Cqe);

	}

	return Ret;
}

/*****************************************************************************/
/**
*
* This function waits for a single asynchronous operation to complete using
* io_uring.
*
* @param	IOInst: IO instance pointer
*
* @return	XAIE_OK on success, XAIE_ERR on failure.
*
* @note		Internal only. Wrapper around XAie_LinuxIO_AsyncWaitNr with
*		Nr set to 1.
*
*******************************************************************************/
static AieRC XAie_LinuxIO_AsyncWait(void *IOInst)
{
	return XAie_LinuxIO_AsyncWaitNr(IOInst, 1);
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
static AieRC XAie_LinuxIO_Write32(void *IOInst, u64 RegOff, u32 Value)
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
		XAIE_ERROR("Register write failed for offset 0x%lx, %d: %s\n",
			RegOff, errno, strerror(errno));
		return XAIE_ERR;
	}

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
static AieRC XAie_LinuxIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	u32 ColClockStatus;
	u64 OffsetAddr = 0U, MemSize = 0U;
	XAie_DevInst *DevInst = LinuxIOInst->DevInst;
	u8 Row = _XAie_GetRowNum(LinuxIOInst, RegOff);
	u8 Col = _XAie_GetColNum(LinuxIOInst, RegOff);
	XAie_LocType Loc = {Row, Col};

	/*
	 * Check bitmap position of the requested tile, If it is
	 * gated tile return XAIE_INVALID_TILE,
	 */
	if(Row != 0U) {
		ColClockStatus = _XAie_GetTileBitPosFromLoc(DevInst,Loc);
		if(!CheckBit(DevInst->DevOps->TilesInUse, ColClockStatus)) {
			XAIE_ERROR("Tile(%d,%d) is gated \n",Col,Row);
			return XAIE_INVALID_TILE;
		}
	}

	u8 TileType = XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
		OffsetAddr = LinuxIOInst->DataMemAddr + XAie_GetTileAddr(DevInst, Row, Col);
		MemSize = OffsetAddr + LinuxIOInst->DataMemSize;
	} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
		OffsetAddr = LinuxIOInst->MemTileMemAddr + XAie_GetTileAddr(DevInst, Row, Col);
		MemSize = OffsetAddr + LinuxIOInst->MemTileMemSize;
	} else {
		/*
		 * Workaround to fix shim tile register out of bound issue
		 */
		OffsetAddr = LinuxIOInst->MemTileMemAddr + XAie_GetTileAddr(DevInst, Row, Col);
		MemSize = OffsetAddr + LinuxIOInst->MemTileMemSize;
	}

	if(RegOff >= OffsetAddr && RegOff <=  MemSize) {
		if((RegOff+sizeof(u32)) > MemSize) {
			XAIE_ERROR(" Reading register failed for offset 0x%lx, [%d, %d], OffsetAddr: 0x%x MemSize: 0x%x\n",
					RegOff, Col, Row, OffsetAddr, MemSize);
			return XAIE_ERR;
		}
	} else if(RegOff < OffsetAddr && ((RegOff+sizeof(u32)) >= OffsetAddr
				&& (RegOff+sizeof(u32)) <=  MemSize)) {
		XAIE_ERROR(" Reading register failed for offset 0x%lx, [%d, %d]\n",
					RegOff, Col, Row);
		return XAIE_ERR;
	}

	*Data = *((u32 *)(LinuxIOInst->RegMap.VAddr + RegOff));

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
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_LinuxIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
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
		XAIE_ERROR("Register write failed for offset 0x%lx, %d: %s\n",
			RegOff, errno, strerror(errno));
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to asynchronously write a masked 32-bit value
* to a register using io_uring.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to write.
* @param	Mask: Mask to be applied to Value.
* @param	Value: 32-bit value to be written.
* @param	AsyncRes: Pointer to async result structure. On error, AsyncRes->res
*			is set to the error code and 0 is returned.
*
* @return	0 on error, positive SQE count on success.
*
* @note		Internal only.
*
******************************************************************************/
static int XAie_LinuxIO_MaskWrite32_Async(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, XAie_AsyncRes *AsyncRes)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	struct aie_reg_args *Args;
	struct io_uring_sqe *Sqe;
	int Ret;

	AsyncRes->io_vec_inuse = 0;
	Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
	if (!Sqe) {
		XAIE_ERROR("Failed to get sqe for async mask write\n");
		AsyncRes->res = -ENOMEM;
		return 0;
	}
	Sqe->opcode = IORING_OP_URING_CMD;
	Sqe->flags |= IOSQE_FIXED_FILE;
	Sqe->cmd_op = AIE_REG_WRITE_CMD;
	Sqe->user_data = (u64)AsyncRes;
	Args = (struct aie_reg_args *)Sqe->cmd;
	*Args = (struct aie_reg_args){
		.op = AIE_REG_WRITE,
		.offset = RegOff,
		.val = Value,
		.mask = Mask,
	};

	Ret = io_uring_submit(&LinuxIOInst->Ring);
	if (Ret < 0) {
		XAIE_ERROR("Failed to submit async mask write: %d\n", Ret);
		AsyncRes->res = Ret;
		return 0;
	}
	return 1;
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
static AieRC XAie_LinuxIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	u32 Count, MinTimeOutUs, RegVal;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	MinTimeOutUs = 200;
	Count = ((u64)TimeOutUs + MinTimeOutUs - 1) / MinTimeOutUs;

	while (Count > 0U) {
		XAie_LinuxIO_Read32(IOInst, RegOff, &RegVal);
		if((RegVal & Mask) == Value) {
			return XAIE_OK;
		}
		usleep(MinTimeOutUs);
		Count--;
	}

	/* Check for the break from timed-out loop */
	XAie_LinuxIO_Read32(IOInst, RegOff, &RegVal);
	if((RegVal & Mask) == Value) {
		return XAIE_OK;
	}

	return XAIE_ERR;
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
* @note		Internal only.
*
*******************************************************************************/
static inline u64 _XAie_GetMemOffset(XAie_LinuxIO *IOInst, u8 TileType, u8 Col,
		u8 Row, u64 MemSize)
{
	u8 AieTileNumRows = IOInst->DevInst->AieTileNumRows;
	u8 MemTileNumRows = IOInst->DevInst->MemTileNumRows;

	if(TileType == XAIEGBL_TILE_TYPE_AIETILE)
		return (u64)(Col * AieTileNumRows * MemSize) +
			((u64)(Row - MemTileNumRows - 1) * MemSize);
	else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE)
		return (u64)(Col * MemTileNumRows * MemSize) +
			((u64)(Row - 1) * MemSize);

	return 0;
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
static void _XAie_CopyDataToMem(u32 *Dest, const u32 *Src, u32 Size)
{
	u32 StartExtraWrds = 0, EndExtraWrds = 0;

	if((u64)Dest & XAIE_128BIT_ALIGN_MASK) {
		StartExtraWrds = ((XAIE_128BIT_ALIGN_MASK + 1) -
				(((u64)Dest) & XAIE_128BIT_ALIGN_MASK)) / 4;
	}

	if(StartExtraWrds > 0) {
		for(u32 i = 0; i < StartExtraWrds && Size > 0;  i++) {
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

	if(Size >= (XAIE_128BIT_ALIGN_MASK + 1) &&
		(((u64)Dest & XAIE_128BIT_ALIGN_MASK) == 0) &&
		(((u64)Src & XAIE_128BIT_ALIGN_MASK) == 0)) {
		memcpy((void *)Dest, (void *)Src,
				(Size - EndExtraWrds) * sizeof(u32));
		Dest += Size - EndExtraWrds;
		Src += Size - EndExtraWrds;
		Size -= Size - EndExtraWrds;
	}

	if(Size > 0) {
		for(u32 i = 0; Size > 0; i++) {
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
	u32 ColClockStatus;
	u64 MemOffset;
	XAie_DevInst *DevInst = IOInst->DevInst;
	u64 RegAddr = _XAie_GetRegAddr(IOInst, RegOff);
	u8 Row = _XAie_GetRowNum(IOInst, RegOff);
	u8 Col = _XAie_GetColNum(IOInst, RegOff);
	XAie_LocType Loc = {Row, Col};
	u8 TileType;

	/*
	 * Check bitmap position of the requested tile, If it is
	 * gated tile return XAIE_INVALID_TILE,
	 */
	if(Row != 0U) {
		ColClockStatus = _XAie_GetTileBitPosFromLoc(DevInst,Loc);
		if(!CheckBit(DevInst->DevOps->TilesInUse, ColClockStatus)) {
			XAIE_ERROR("Tile(%d,%d) is gated \n",Col,Row);
			return (u32 *)XAIE_INVALID_TILE;
		}
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
		u64 MemRange = IOInst->MemTileMemAddr + IOInst->MemTileMemSize;
		if(((RegAddr + Size) < (MemRange)) &&
				(RegAddr >= IOInst->MemTileMemAddr)) {
			MemOffset = _XAie_GetMemOffset(IOInst, TileType, Col, Row,
					IOInst->MemTileMemSize);
			VirtAddr = (u32 *)((char *)IOInst->MemTileMem.VAddr +
					MemOffset + RegAddr);

			return VirtAddr;
		}
	}

	if(((RegAddr + Size) < (IOInst->ProgMemAddr + IOInst->ProgMemSize)) &&
			(RegAddr >= IOInst->ProgMemAddr)) {
		/* Handle program memory block write */
		MemOffset = _XAie_GetMemOffset(IOInst, TileType, Col, Row,
				IOInst->ProgMemSize);
		VirtAddr = (u32 *)((char *) IOInst->ProgMem.VAddr + MemOffset +
				RegAddr - IOInst->ProgMemAddr);
	} else if(((RegAddr + Size) < (IOInst->DataMemAddr +
					IOInst->DataMemSize)) &&
			(RegAddr >= IOInst->DataMemAddr)) {
		/* Handle data memory block write */
		MemOffset = _XAie_GetMemOffset(IOInst, TileType, Col, Row,
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
static AieRC XAie_LinuxIO_BlockWrite32(void *IOInst, u64 RegOff,
		const u32 *Data, u32 Size)
{
	XAie_LinuxIO *Inst = (XAie_LinuxIO *)IOInst;
	u32 *VirtAddr;
	AieRC RC;

	/* Handle PM and DM sections */
	VirtAddr =  _XAie_GetVirtAddrFromOffset(Inst, RegOff, Size);
	if(VirtAddr != NULL && VirtAddr != (u32 *)XAIE_INVALID_TILE) {
		_XAie_CopyDataToMem(VirtAddr, Data, Size);
		return XAIE_OK;
	}else if(VirtAddr == (u32 *)XAIE_INVALID_TILE) {
		XAIE_ERROR("Tile is gated \n");
		return XAIE_ERR;
	}
	/* Handle other registers */
	for(u32 i = 0; i < Size; i++) {
		RC = XAie_LinuxIO_Write32(IOInst, RegOff + i * 4U, *Data);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Block Write Failed!\n");
			return RC;
		}
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
static AieRC XAie_LinuxIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data,
		u32 Size)
{
	XAie_LinuxIO *Inst = (XAie_LinuxIO *)IOInst;
	u32 *VirtAddr;

	/* Handle PM and DM sections */
	VirtAddr =  _XAie_GetVirtAddrFromOffset(Inst, RegOff, Size);
	if(VirtAddr != NULL && VirtAddr != (u32 *)XAIE_INVALID_TILE) {
		for(u32 i = 0; i < Size; i++) {
			*VirtAddr++ = Data;
		}
		return XAIE_OK;
	}else if(VirtAddr == (u32 *)XAIE_INVALID_TILE) {
		XAIE_ERROR("Tile is gated \n");
		return XAIE_ERR;
	}

	/* Handle other registers */
	for(u32 i = 0; i < Size; i++) {
		XAie_LinuxIO_Write32(IOInst, RegOff + i * 4U, Data);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to attach the external memory to device
*
* @param	MemInst: Memory instance pointer.
* @param	MemHandle: dmabuf fd
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_LinuxMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	XAie_LinuxMem *LinuxMemInst;
	XAie_DevInst *DevInst;
	XAie_LinuxIO *IOInst;
	int Ret;

	if ((MemInst == NULL) ||
	    (MemInst->DevInst == NULL)) {
		XAIE_ERROR("Invalid memory instance or backend handle for attach\n");
		return XAIE_ERR;
	}
	MemInst->BackendHandle = malloc(sizeof(XAie_LinuxMem));
	if (MemInst->BackendHandle == NULL) {
		XAIE_ERROR("Failed to allocate memory for LinuxMemInst\n");
		return XAIE_ERR;
	}

	DevInst = MemInst->DevInst;
	IOInst = (XAie_LinuxIO *)DevInst->IOInst;
	LinuxMemInst = (XAie_LinuxMem *)MemInst->BackendHandle;
	LinuxMemInst->BufferFd = (int)MemHandle;
	Ret = ioctl(IOInst->PartitionFd, AIE_ATTACH_DMABUF_IOCTL, MemHandle);
	if (Ret < 0) {
		XAIE_ERROR("Failed to attach dmabuf fd %d, %d: %s\n",
			(int)MemHandle, errno, strerror(errno));
		free(MemInst->BackendHandle);
		MemInst->BackendHandle = NULL;
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to asynchronously attach the allocated memory to
* the device using io_uring.
*
* @param	MemInst: Memory instance pointer.
* @param	MemHandle: Handle to the allocated memory (DMA buffer fd).
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking.
*
* @return	Number of SQEs submitted on success(0 in this case, or < 0 on failure.
*
* @note		Internal only. Uses io_uring for async DMA buffer attachment.
*
*******************************************************************************/
static int XAie_LinuxMemAttachAsync(XAie_MemInst *MemInst, u64 MemHandle,
				    XAie_AsyncRes *AsyncRes)
{
	AsyncRes->res = AsyncRes->res2 = AsyncRes->res3 = MemHandle;

	return 0;
}

/*****************************************************************************/
/**
*
* This is the memory function to detach the memory from device
*
* @param	MemInst: Memory instance pointer.
*
* @return	XAIE_OK for success
*
* @note		None.
*
*******************************************************************************/
static AieRC XAie_LinuxMemDetach(XAie_MemInst *MemInst)
{
	XAie_DevInst *DevInst = MemInst->DevInst;
	XAie_LinuxIO *IOInst = (XAie_LinuxIO *)DevInst->IOInst;
	XAie_LinuxMem *LinuxMemInst;
	int Ret;

	LinuxMemInst = (XAie_LinuxMem *)MemInst->BackendHandle;
	Ret = ioctl(IOInst->PartitionFd, AIE_DETACH_DMABUF_IOCTL,
		    LinuxMemInst->BufferFd);
	if (Ret < 0) {
		XAIE_ERROR("Failed to detach dmabuf fd %d, %d: %s\n",
			*((int *)MemInst->BackendHandle), errno, strerror(errno));
		return XAIE_ERR;
	}
	free(MemInst->BackendHandle);
	MemInst->BackendHandle = NULL;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory function to asynchronously detach the memory from device
* using io_uring. The DMA buffer fd is passed to the kernel via the SQE command
* buffer. On success, the backend memory handle is freed.
*
* @param	MemInst: Memory instance pointer.
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking. On error, AsyncRes->res is set to the
*			corresponding error code.
*
* @return	Number of SQEs submitted on success(0 in this case), or < 0 on failure.
*
* @note		Internal only. Uses io_uring for async DMA buffer detachment.
*
*******************************************************************************/
static int XAie_LinuxMemDetachAsync(XAie_MemInst *MemInst, XAie_AsyncRes *AsyncRes)
{
	(void)MemInst;
	AsyncRes->res = AsyncRes->res2 = AsyncRes->res3 = 0;
	return 0;
}

/*****************************************************************************/
/**
*
* This API updates the address in shim dma bd using the linux kernel driver.
*
* @param	IOInst: IO instance pointer
* @param	Args: Shim dma arguments pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*		This function will update SHIM DMA BD with dmabuf.
*		The address field in the DMA buffer descriptor is the offset to
*		the start of the dmabuf specified by the memory object.
*
*******************************************************************************/
static AieRC _XAie_LinuxIO_UpdateShimDmaBdAddrOff(void *IOInst,
						 XAie_ShimDmaBdArgs *Args)
{
	XAie_LinuxMem *LinuxMemInst =
		(XAie_LinuxMem *)Args->MemInst->BackendHandle;
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	struct aie_dmabuf_bd_args ShimArgs;
	int Ret;

	if (LinuxMemInst == XAIE_NULL) {
		XAIE_ERROR("Failed to configure shim dma bd, invalid bd MemInst.\n");
		return XAIE_INVALID_ARGS;
	}

	ShimArgs.bd = Args->BdWords;
	ShimArgs.loc.row = Args->Loc.Row;
	ShimArgs.loc.col = Args->Loc.Col;
	ShimArgs.bd_id = Args->BdNum;
	ShimArgs.buf_fd = LinuxMemInst->BufferFd;

	Ret = ioctl(LinuxIOInst->PartitionFd,
		    AIE_UPDATE_SHIMDMA_DMABUF_BD_ADDR_IOCTL,
		    &ShimArgs);
	if (Ret < 0) {
		XAIE_ERROR("Failed to update shim dma bd addr, %d: %s Tile(%u,%u)\n",
			errno, strerror(errno),ShimArgs.loc.col, ShimArgs.loc.row);
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is function to configure shim dma using the linux kernel driver.
*
* @param	IOInst: IO instance pointer
* @param	Args: Shim dma arguments pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*		This function will set SHIM DMA BD with dmabuf. It will check
*		if the memory object is specified in the DMA descriptor
*		If there is no memory object specified it will return error.
*		The address field in the DMA buffer descriptor is the offset to
*		the start of the dmabuf specified by the memory object.
*
*******************************************************************************/
static AieRC _XAie_LinuxIO_ConfigShimDmaBd(void *IOInst,
		XAie_ShimDmaBdArgs *Args)
{
	int Ret;

	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;

	if (Args->MemInst == XAIE_NULL) {
		struct aie_dma_bd_args ShimArgs;

		ShimArgs.bd = Args->BdWords;
		ShimArgs.data_va = Args->VAddr;
		ShimArgs.loc.row = Args->Loc.Row;
		ShimArgs.loc.col = Args->Loc.Col;
		ShimArgs.bd_id = Args->BdNum;

		Ret = ioctl(LinuxIOInst->PartitionFd, AIE_SET_SHIMDMA_BD_IOCTL,
				&ShimArgs);
	} else {
		struct aie_dmabuf_bd_args ShimArgs;
		XAie_LinuxMem *LinuxMemInst =
			(XAie_LinuxMem *)Args->MemInst->BackendHandle;

		if (LinuxMemInst == XAIE_NULL) {
			XAIE_ERROR("Failed to configure shim dma bd, invalid bd MemInst.\n");
			return XAIE_INVALID_ARGS;
		}

		ShimArgs.bd = Args->BdWords;
		ShimArgs.loc.row = Args->Loc.Row;
		ShimArgs.loc.col = Args->Loc.Col;
		ShimArgs.bd_id = Args->BdNum;
		ShimArgs.buf_fd = LinuxMemInst->BufferFd;

		Ret = ioctl(LinuxIOInst->PartitionFd, AIE_SET_SHIMDMA_DMABUF_BD_IOCTL,
				&ShimArgs);
	}

	if(Ret != 0) {
		XAIE_ERROR("Failed to configure shim dma bd, %d: %s Tile(%u,%u)\n",
			errno, strerror(errno),Args->Loc.Col,Args->Loc.Row);
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is function to get configuration of shim dma to add into transaction buffer
*
* @param	Args: Shim dma arguments pointer.
*
* @return	Structure pointer on success, NULL on failure.
*
*******************************************************************************/
static void* XAie_LinuxIO_GetShimDmaBdConfig(XAie_ShimDmaBdArgs *Args)
{
	if (Args->MemInst == XAIE_NULL) {
		struct aie_dma_bd_args *ShimArgs =
		(struct aie_dma_bd_args *)malloc(sizeof(struct aie_dma_bd_args));

		if(ShimArgs == XAIE_NULL) {
			XAIE_ERROR("Memory allocation for SHIM DMA Config failed\n");
			return XAIE_NULL;
		}

		u32 *BdWords = (u32 *)malloc(sizeof(u32) * Args->NumBdWords);
		if(BdWords == XAIE_NULL) {
			XAIE_ERROR("Memory allocation failed\n");
			free(ShimArgs);
			return XAIE_NULL;
		}

		BdWords = memcpy((void *)BdWords, (void *)Args->BdWords,
					sizeof(u32) * Args->NumBdWords);
		ShimArgs->bd = (u32 *)BdWords;
		ShimArgs->data_va = Args->VAddr;
		ShimArgs->loc.row = Args->Loc.Row;
		ShimArgs->loc.col = Args->Loc.Col;
		ShimArgs->bd_id = Args->BdNum;

		return (void *)ShimArgs;
	} else {
		XAie_LinuxMem *LinuxMemInst =
			(XAie_LinuxMem *)Args->MemInst->BackendHandle;

		if (LinuxMemInst == XAIE_NULL) {
			XAIE_ERROR("Failed to get shim dma bd, "
						"invalid bd MemInst.\n");
			return XAIE_NULL;
		}

		struct aie_dmabuf_bd_args *ShimArgs =
	                (struct aie_dmabuf_bd_args *)malloc(
				sizeof(struct aie_dmabuf_bd_args));
		if(ShimArgs == XAIE_NULL) {
			XAIE_ERROR("Memory allocation for SHIM "
					"DMA Config failed\n");
			return XAIE_NULL;
		}

		u32 *BdWords = (u32 *)malloc(sizeof(u32) * Args->NumBdWords);
		if(BdWords == XAIE_NULL) {
			XAIE_ERROR("Memory allocation failed\n");
			free(ShimArgs);
			return XAIE_NULL;
		}

		BdWords = memcpy((void *)BdWords, (void *)Args->BdWords,
					sizeof(u32) * Args->NumBdWords);
		ShimArgs->bd = (u32 *)BdWords;
		ShimArgs->loc.row = Args->Loc.Row;
		ShimArgs->loc.col = Args->Loc.Col;
		ShimArgs->bd_id = Args->BdNum;
		ShimArgs->buf_fd = LinuxMemInst->BufferFd;

		return (void *)ShimArgs;
	}
}

/*****************************************************************************/
/**
*
* This is function to request AI engine tiles
*
* @param	IOInst: IO instance pointer
* @param	Args: Tiles array argument
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_LinuxIO_RequestTiles(void *IOInst,
		XAie_BackendTilesArray *Args)
{
	struct aie_tiles_array TilesArray;
	int Ret;

	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;

	TilesArray.num_tiles = Args->NumTiles;
	TilesArray.locs = XAIE_NULL;
	if (TilesArray.num_tiles != 0) {
		TilesArray.locs = malloc(TilesArray.num_tiles *
					 sizeof(TilesArray.locs[0]));
		if (TilesArray.locs == XAIE_NULL) {
			XAIE_ERROR("request tiles, failed to allocate memory for tiles. NumTile:%d\n",TilesArray.num_tiles);
			return XAIE_ERR;
		}

		for (u32 i = 0; i < TilesArray.num_tiles; i++) {
			TilesArray.locs[i].col = Args->Locs[i].Col;
			TilesArray.locs[i].row = Args->Locs[i].Row;
		}
	}

	/* Clear the TilesInuse bitmap to reflect the current status */
	for(u32 C = 0; C < LinuxIOInst->NumCols; C++) {
		XAie_LocType Loc;
		u32 ColClockStatus;

		Loc = XAie_TileLoc(C, 1);
		ColClockStatus = _XAie_GetTileBitPosFromLoc(LinuxIOInst->DevInst, Loc);

		_XAie_ClrBitInBitmap(LinuxIOInst->DevInst->DevOps->TilesInUse,
				ColClockStatus, LinuxIOInst->DevInst->NumRows - 1);
	}

	Ret = ioctl(LinuxIOInst->PartitionFd, AIE_REQUEST_TILES_IOCTL,
			&TilesArray);
	free(TilesArray.locs);
	if(Ret != 0) {
		XAIE_ERROR("Failed to request tiles, %d: %s\n",
			errno, strerror(errno));
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is function to release AI engine tiles
*
* @param	IOInst: IO instance pointer
* @param	Args: Tiles array argument
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_LinuxIO_ReleaseTiles(void *IOInst,
		XAie_BackendTilesArray *Args)
{
	struct aie_tiles_array TilesArray;
	int Ret;

	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;

	TilesArray.num_tiles = Args->NumTiles;
	TilesArray.locs = XAIE_NULL;
	if (TilesArray.num_tiles != 0) {
		TilesArray.locs = malloc(TilesArray.num_tiles *
					 sizeof(TilesArray.locs[0]));
		if (TilesArray.locs == XAIE_NULL) {
			XAIE_ERROR("release tiles, failed to allocate memory for tiles\n");
			return XAIE_ERR;
		}

		for (u32 i = 0; i < TilesArray.num_tiles; i++) {
			TilesArray.locs[i].col = Args->Locs[i].Col;
			TilesArray.locs[i].row = Args->Locs[i].Row;
		}
	}

	Ret = ioctl(LinuxIOInst->PartitionFd, AIE_RELEASE_TILES_IOCTL,
			&TilesArray);
	free(TilesArray.locs);
	if(Ret != 0) {
		XAIE_ERROR("Failed to request tiles, %d: %s\n",
			errno, strerror(errno));
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is function enables/disables column clock control register to request AI engine tiles
*
* @param	IOInst: IO instance pointer
* @param	Args: Tiles array argument
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_LinuxIO_SetColumnClock(void *IOInst,
		XAie_BackendColumnReq *Args)
{
	struct aie_column_args ColumnReq;
	int Ret;

	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;

	ColumnReq.start_col = Args->StartCol;
	ColumnReq.num_cols = Args->NumCols;
	ColumnReq.enable = Args->Enable;

	Ret = ioctl(LinuxIOInst->PartitionFd, AIE_SET_COLUMN_CLOCK_IOCTL,
			&ColumnReq);
	if(Ret != 0) {
		XAIE_ERROR("Failed to request tiles, %d: %s StartCol:%u NumCol:%u\n",
			errno, strerror(errno),Args->StartCol,Args->NumCols);
		return XAIE_ERR;
	}

	return XAIE_OK;
}

#ifdef _POSIX_C_SOURCE
/*****************************************************************************/
/**
* The API is a signal callback to calculate the core tile utilization and store
* in a floating point variable.
*
* @param	IOInst: IO instance pointer
* @param	PerfInst: Performance instance.
*
* @return	None.
*
* @note		Internal only.
*
*******************************************************************************/
void _XAie_LinuxIO_UtilCalculation(int SignalNum, siginfo_t *SignalInfo,
		void *Arg)
{
	XAie_LinuxIO *LinuxIO = (XAie_LinuxIO *)IOInstLinux;

	(void)Arg;
	(void)SignalNum;

	int Ret;
	AieRC RC;
	u32 CounterVal;
	float ActiveCycle, TotalCycle;
	XAie_PerfInst *PerfInst = (XAie_PerfInst *) SignalInfo->si_value.sival_ptr;

	for(uint32_t Index = 0; Index < PerfInst->UtilSize; Index++) {
		for(uint8_t Cycle = 0; Cycle < XAIE_PERF_CORE_NUM_CYCLES;
		    Cycle++) {
			RC = XAie_PerfCounterGet(LinuxIO->DevInst,
					PerfInst->Util[Index].Loc, XAIE_CORE_MOD,
					PerfInst->Util[Index].PerfCnt[Cycle],
					&CounterVal);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Error retrieving the number of cycles\n");
				return;
			}
			if(Cycle == XAIE_CORE_ACTIVE_CYCLE) {
				ActiveCycle = (float) CounterVal;
				RC = XAie_EventGenerate(LinuxIO->DevInst,
						PerfInst->Util[Index].Loc,
						XAIE_CORE_MOD,
						XAIE_EVENT_USER_EVENT_1_CORE);
			} else if(Cycle == XAIE_CORE_TOTAL_CYCLE) {
				TotalCycle = (float) CounterVal;
			}
		}
		PerfInst->Util[Index].KernelUtil = (ActiveCycle / TotalCycle)*100U;
	}

	for(uint32_t Index = 0U; Index < PerfInst->UtilSize; Index++) {
		struct aie_rsc Rsc = {0};
		Rsc.loc.row = PerfInst->Util[Index].Loc.Row;
		Rsc.loc.col = PerfInst->Util[Index].Loc.Col;
		Rsc.mod = XAIE_CORE_MOD;
		Rsc.type = AIE_RSCTYPE_PERF;
		for(uint8_t Counter = 0U; Counter < 2U; Counter++) {
			Rsc.id = PerfInst->Util[Index].PerfCnt[Counter];
			Ret = ioctl(LinuxIO->PartitionFd,
					AIE_RSC_RELEASE_IOCTL, &Rsc);
			if(Ret != 0U) {
				XAIE_WARN("Failed to release resource %u\n",
						Rsc.type);
			}

		}

	}

	/*
	 * Stores the size of the Util array in bytes.
	 */
	PerfInst->UtilSize *= sizeof(XAie_Occupancy);
	timer_delete(TimerID);
}

/*****************************************************************************/
/**
* The API captures core tile utilization over a user-defined period.
*
* @param	IOInst: IO instance pointer
* @param	PerfInst: Performance instance.
*
* @return	XAIE_OK on success, error otherwise.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_LinuxIO_PerfUtilization(void *IOInst, XAie_PerfInst *PerfInst)
{
	int Ret;
	AieRC RC;
	struct sigaction SignalAction;
	struct sigevent SignalEvent;
	static struct itimerspec TimerSpec;
	const XAie_CoreMod* CoreMod;
	struct aie_rsc Rscs[2];
	XAie_LocType Loc;
	XAie_Events StartEvent, StopEvent, ResetEvent;
	u32 CoreStatus, Index;
	struct aie_rsc_req_rsp RscReq = {0};
	struct aie_rsc Rsc;


	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;

	UserInst = PerfInst;
	IOInstLinux = (void *) LinuxIOInst;

	/*
	 * Registering signal callback to calculate utilization as floating
	 * point cannot be calculated in linux kernel driver.
	 */
	sigemptyset(&SignalAction.sa_mask);
	SignalAction.sa_flags = SA_SIGINFO;
	SignalAction.sa_sigaction = _XAie_LinuxIO_UtilCalculation;
	sigaction (SIGRTMIN, &SignalAction, NULL);

	/*
	 * Setting up the timer to fire at the end of user-defined time.
	 */
	SignalEvent.sigev_notify = SIGEV_SIGNAL;
	SignalEvent.sigev_signo = SIGRTMIN;
	SignalEvent.sigev_value.sival_ptr = UserInst;
	Ret = timer_create(CLOCK_REALTIME, &SignalEvent, &TimerID);
	if(Ret < 0) {
		XAIE_ERROR("Error creating the timer %d: %s\n", errno,
			   strerror(errno));
		return XAIE_ERR;
	}

	TimerSpec.it_value.tv_sec = (PerfInst->TimeInterval_ms / 1000) % 10U;
	TimerSpec.it_value.tv_nsec = ((float)PerfInst->TimeInterval_ms / 1000 -
			TimerSpec.it_value.tv_sec) * 1000U * 1000000U;

	/*
	 * Populates all the tiles enabled and in use.
	 * Reserves the performance counters for the tiles in the Util array.
	 */
	CoreMod = LinuxIOInst->DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;
	Index = 0U;
	for(Loc.Col = PerfInst->Range->Start; Loc.Col < PerfInst->Range->Num;
			Loc.Col++) {
		for(Loc.Row = LinuxIOInst->DevInst->AieTileRowStart; Loc.Row <
				(LinuxIOInst->DevInst->AieTileRowStart +
				 LinuxIOInst->DevInst->AieTileNumRows);
				 Loc.Row++) {
			RC = XAie_CoreGetStatus(LinuxIOInst->DevInst, Loc,
					&CoreStatus);
			XAIE_DBG("Corestatus[%d, %d]: %d\n", Loc.Col, Loc.Row, CoreStatus);
			if(XAie_GetField(CoreStatus, CoreMod->CoreSts->En.Lsb,
				CoreMod->CoreSts->En.Mask) &&
				_XAie_PmIsTileRequested(LinuxIOInst->DevInst,
							Loc)) {
				PerfInst->Util[Index].Loc = Loc;
				RscReq.req.loc.col = PerfInst->Util[Index].Loc.Col;
				RscReq.req.loc.row = PerfInst->Util[Index].Loc.Row;
				RscReq.req.mod = XAIE_CORE_MOD;
				RscReq.req.type = AIE_RSCTYPE_PERF;
				RscReq.req.num_rscs = 2U;
				RscReq.rscs = (__u64)&Rscs;
				Ret = ioctl(LinuxIOInst->PartitionFd,
					    AIE_RSC_REQ_IOCTL, &RscReq);
				if(Ret != 0U) {
					XAIE_WARN("Failed to request performance counter %u\n",
						RscReq.req.type);
					return XAIE_ERR;
				}
				for(__u32 Counter = 0U;
				    Counter < RscReq.req.num_rscs; Counter++) {
					PerfInst->Util[Index].PerfCnt[Counter] =
						Rscs[Counter].id;
				}

				for(u32 Id = 0U;
					Id < XAIE_OCCUPANCY_USER_EVENT_NUM;
					Id++) {
					Rsc.loc.col = PerfInst->Util[Index].Loc.Col;
					Rsc.loc.row = PerfInst->Util[Index].Loc.Row;
					Rsc.mod = XAIE_CORE_MOD;
					Rsc.type = AIE_RSCTYPE_USEREVENT;
					Rsc.id = Id;
					Ret = ioctl(LinuxIOInst->PartitionFd,
						    AIE_RSC_REQ_SPECIFIC_IOCTL,
						    &Rsc);
					if(Ret != 0) {
						XAIE_WARN("Failed to request user event %u, %d: %s\n",
							   Rsc.type, errno,
							   strerror(errno));
						return XAIE_ERR;
					}
				}
				Index++;

			}
		}
	}

	PerfInst->UtilSize = Index;
	XAIE_DBG("PerfInst->UtilSize set to %d\n", Index);

	for(uint32_t UIndex = 0U; UIndex < PerfInst->UtilSize; UIndex++) {
		for(uint8_t Cycle = 0U; Cycle < XAIE_PERF_CORE_NUM_CYCLES;
		    Cycle++) {
			if(Cycle == XAIE_CORE_ACTIVE_CYCLE) {
				StartEvent = XAIE_EVENT_ACTIVE_CORE;
				StopEvent = XAIE_EVENT_DISABLED_CORE;
			} else if(Cycle == XAIE_CORE_TOTAL_CYCLE) {
				StartEvent = XAIE_EVENT_USER_EVENT_0_CORE;
				StopEvent = XAIE_EVENT_USER_EVENT_1_CORE;
			}

			ResetEvent = XAIE_EVENT_USER_EVENT_0_CORE;

			RC = XAie_PerfCounterControlSet(LinuxIOInst->DevInst,
				PerfInst->Util[UIndex].Loc, XAIE_CORE_MOD,
				PerfInst->Util[UIndex].PerfCnt[Cycle],
				StartEvent, StopEvent);
			RC |= XAie_PerfCounterResetControlSet(LinuxIOInst->DevInst,
				PerfInst->Util[UIndex].Loc, XAIE_CORE_MOD,
				PerfInst->Util[UIndex].PerfCnt[Cycle], ResetEvent);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Performance counter start, stop or reset event setup failed for (%u, %u) at Index: %u!\n",
						PerfInst->Util[UIndex].Loc.Row,
						PerfInst->Util[UIndex].Loc.Col,
						UIndex);
				return RC;
			}

		}

	}

	/*
	 * Resets active cycle & total cycle performance counter and starts the
	 * total cycle performance counter.
	 * Since the core occupancy capture is time-sensitive performance
	 * counter setup and reset is split as separate runs.
	 */
	for(uint32_t UIndex = 0U; UIndex < PerfInst->UtilSize; UIndex++) {
		RC = XAie_EventGenerate(LinuxIOInst->DevInst,
						PerfInst->Util[UIndex].Loc,
						XAIE_CORE_MOD,
						XAIE_EVENT_USER_EVENT_0_CORE);
	}

	/*
	 * Enables the timer.
	 */
	Ret = timer_settime(TimerID, 0, &TimerSpec, NULL);
	if(Ret < 0) {
		XAIE_ERROR("Error starting the timer %d: %s\n", errno,
			   strerror(errno));
		return XAIE_ERR;
	}

	return XAIE_OK;
}

#else /* _POSIX_C_SOURCE */
/*****************************************************************************/
/**
* The API captures core tile utilization over a user-defined period.
*
* @param	IOInst: IO instance pointer
* @param	PerfInst: Performance instance.
*
* @return	XAIE_ERR as posix is not enabled.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_LinuxIO_PerfUtilization(void *IOInst, XAie_PerfInst *PerfInst)
{
	(void)IOInst;
	(void)PerfInst;

	XAIE_ERROR("POSIX is not enabled. Aborting the occupancy collection!\n");
	return XAIE_ERR;

}
#endif /* _POSIX_C_SOURCE */

/*****************************************************************************/
/**
* The API clears partition context.
*
*
* @param	IOInst: IO instance pointer
*
* @return	XAIE_OK on success
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_LinuxIO_PartClearContext(XAie_LinuxIO *IOInst)
{
	int Ret;

	Ret = ioctl(IOInst->PartitionFd, AIE_PARTITION_CLR_CONTEXT_IOCTL);
	if(Ret != 0) {
		XAIE_ERROR("Failed to clear partition context, %d: %s\n",
			errno, strerror(errno));
		return XAIE_ERR;
	}

	return XAIE_OK;
}

static int XAie_LinuxIO_PartitionInitAsync(void *IOInst, XAie_PartInitOpts *Opts,
					   XAie_AsyncRes *AsyncRes)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *) IOInst;
	struct io_uring_sqe *Sqe;
	struct aie_partition_init_args *InitArgs;
	int ret;

	if ((Opts != NULL) && (Opts->NumUseTiles != 0) && (Opts->Locs !=NULL)) {
		if (_XAie_LinuxIO_IoVecIsFull(LinuxIOInst)) {
			XAIE_ERROR("io_uring iovec array is full, cannot submit async partition init\n");
			AsyncRes->res = -ENOMEM;
			return 0;
		}
		if ((Opts->NumUseTiles * sizeof(struct aie_location) >
		    (long unsigned int)getpagesize())) {
			XAIE_ERROR("Number of tiles requested exceeds iovec buffer size for async partition init\n");
			AsyncRes->res = -EINVAL;
			return 0;
		}
	}

	Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
	if (Sqe == NULL) {
		XAIE_ERROR("Failed to get sqe for async partition init\n");
		AsyncRes->res = -ENOMEM;
		return 0;
	}
	Sqe->opcode = IORING_OP_URING_CMD;
	Sqe->flags |= IOSQE_FIXED_FILE;
	Sqe->cmd_op = AIE_PARTITION_INIT_IOCTL;
	Sqe->user_data = (u64)AsyncRes;
	InitArgs = (struct aie_partition_init_args *)Sqe->cmd;

	if (Opts != NULL) {
		InitArgs->init_opts = Opts->InitOpts;
		if (Opts->NumUseTiles == 0 || Opts->Locs == NULL) {
			InitArgs->num_tiles = 0;
			InitArgs->locs = NULL;
		} else {
			Sqe->buf_index = _XAie_LinuxIO_IoVecIncHead(LinuxIOInst);
			InitArgs->num_tiles = Opts->NumUseTiles;
			InitArgs->locs = LinuxIOInst->IoVecs[Sqe->buf_index].iov_base;
			AsyncRes->io_vec_inuse = 1;

			for (__u32 i = 0; i < InitArgs->num_tiles; i++) {
				InitArgs->locs[i].col = (__u32)Opts->Locs[i].Col;
				InitArgs->locs[i].row = (__u32)Opts->Locs[i].Row;
			}
		}
	} else {
		InitArgs->init_opts = XAIE_PART_INIT_OPT_DEFAULT;
		InitArgs->num_tiles = 0;
		InitArgs->locs = NULL;
	}

	/* Clear the TilesInuse bitmap to reflect the current status */
	for(u32 C = 0; C < LinuxIOInst->DevInst->NumCols; C++) {
		XAie_LocType Loc;
		u32 ColClockStatus;

		Loc = XAie_TileLoc(C, 1);
		ColClockStatus = _XAie_GetTileBitPosFromLoc(LinuxIOInst->DevInst, Loc);

		_XAie_ClrBitInBitmap(LinuxIOInst->DevInst->DevOps->TilesInUse,
				     ColClockStatus, LinuxIOInst->DevInst->NumRows - 1);
	}

	if (InitArgs->locs == NULL) {
		u32 StartBit, NumTiles;

		NumTiles = (u32)(LinuxIOInst->DevInst->NumCols * (LinuxIOInst->DevInst->NumRows - 1U));
		/* Loc is NULL, it suggests all tiles are requested */
		StartBit = _XAie_GetTileBitPosFromLoc(LinuxIOInst->DevInst,
				XAie_TileLoc(0, 1));
		_XAie_SetBitInBitmap(LinuxIOInst->DevInst->DevOps->TilesInUse, StartBit,NumTiles);
	} else {
		for(u32 i = 0; i < InitArgs->num_tiles; i++) {
			u32 Bit;

			if(InitArgs->locs[i].row == 0U) {
				continue;
			}

			/*
			 * If a tile is ungated, the rows below it are
			 * ungated.
			 */
			Bit = _XAie_GetTileBitPosFromLoc(LinuxIOInst->DevInst,
							 XAie_TileLoc(InitArgs->locs[i].col, 1));
			_XAie_SetBitInBitmap(LinuxIOInst->DevInst->DevOps->TilesInUse,Bit,
					     InitArgs->locs[i].row);
		}
	}
	ret = io_uring_submit(&LinuxIOInst->Ring);
	if (ret < 0) {
		XAIE_ERROR("Failed to submit async partition init: %d\n", ret);
		AsyncRes->res = ret;
		if (AsyncRes->io_vec_inuse) {
			_XAie_LinuxIO_IoVecDecHead(LinuxIOInst);
		}
		return 0;
	}

	return 1;
}

/*****************************************************************************/
/**
*
* This function asynchronously tears down the AI engine partition using
* io_uring. It submits the partition teardown ioctl as an io_uring command.
*
* @param	IOInst: IO instance pointer.
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking. On error, AsyncRes->res is set to the
*			corresponding error code.
*
* @return	Number of SQEs submitted on success, or 0 on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static int XAie_LinuxIO_TeardownPartAsync(void *IOInst,
					  XAie_AsyncRes *AsyncRes)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	struct io_uring_sqe *Sqe;
	int ret;

	AsyncRes->io_vec_inuse = 0;
	Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
	if (Sqe == NULL) {
		XAIE_ERROR("Failed to get sqe for async partition teardown\n");
		AsyncRes->res = -ENOMEM;
		return 0;
	}
	Sqe->opcode = IORING_OP_URING_CMD;
	Sqe->flags |= IOSQE_FIXED_FILE;
	Sqe->cmd_op = AIE_PARTITION_TEAR_IOCTL;
	Sqe->user_data = (u64)AsyncRes;

	ret = io_uring_submit(&LinuxIOInst->Ring);
	if (ret < 0) {
		XAIE_ERROR("Failed to submit async partition teardown: %d\n",
			   ret);
		AsyncRes->res = ret;
		return 0;
	}

	return 1;
}

/*****************************************************************************/
/**
*
* This function asynchronously clears the AI engine partition context using
* io_uring. It submits the partition clear context ioctl as an io_uring
* command.
*
* @param	IOInst: IO instance pointer.
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking. On error, AsyncRes->res is set to the
*			corresponding error code.
*
* @return	Number of SQEs submitted on success, or 0 on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static int XAie_LinuxIO_PartClearContextAsync(void *IOInst,
					      XAie_AsyncRes *AsyncRes)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	struct io_uring_sqe *Sqe;
	int ret;

	AsyncRes->io_vec_inuse = 0;
	Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
	if (Sqe == NULL) {
		XAIE_ERROR("Failed to get sqe for async partition clear context\n");
		AsyncRes->res = -ENOMEM;
		return 0;
	}
	Sqe->opcode = IORING_OP_URING_CMD;
	Sqe->flags |= IOSQE_FIXED_FILE;
	Sqe->cmd_op = AIE_PARTITION_CLR_CONTEXT_IOCTL;
	Sqe->user_data = (u64)AsyncRes;

	ret = io_uring_submit(&LinuxIOInst->Ring);
	if (ret < 0) {
		XAIE_ERROR("Failed to submit async partition clear context: %d\n",
			   ret);
		AsyncRes->res = ret;
		return 0;
	}

	return 1;
}

/*****************************************************************************/
/**
* This API initializes the AI engine partition
*
* @param	IOInst: IO Instance pointer.
* @param	Opts: Initialization options.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		If Opts is NULL, then default partition initialization
* 		options are used.
*
* @note		Internal only.
*******************************************************************************/
AieRC _XAie_LinuxIO_InitPart(void *IOInst, XAie_PartInitOpts *Opts)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *) IOInst;
	int Ret;
	struct aie_partition_init_args InitArgs;

	if (Opts != NULL) {
		InitArgs.init_opts = Opts->InitOpts;
		InitArgs.num_tiles = Opts->NumUseTiles;
		InitArgs.locs = malloc(InitArgs.num_tiles *
				sizeof(struct aie_location));
		if (InitArgs.locs == XAIE_NULL) {
			XAIE_ERROR("failed to allocate memory for tiles\n");
			return XAIE_ERR;
		}
		for (__u32 i = 0; i < InitArgs.num_tiles; i++) {
			InitArgs.locs[i].col = (__u32)Opts->Locs[i].Col;
			InitArgs.locs[i].row = (__u32)Opts->Locs[i].Row;
		}
	} else {
		InitArgs.init_opts = XAIE_PART_INIT_OPT_DEFAULT;
		InitArgs.num_tiles = 0;
		InitArgs.locs = XAIE_NULL;
	}

	/* Clear the TilesInuse bitmap to reflect the current status */
	for(u32 C = 0; C < LinuxIOInst->DevInst->NumCols; C++) {
		XAie_LocType Loc;
		u32 ColClockStatus;

		Loc = XAie_TileLoc(C, 1);
		ColClockStatus = _XAie_GetTileBitPosFromLoc(LinuxIOInst->DevInst, Loc);

		_XAie_ClrBitInBitmap(LinuxIOInst->DevInst->DevOps->TilesInUse,
				ColClockStatus, LinuxIOInst->DevInst->NumRows - 1);
	}

	if (InitArgs.locs == NULL) {
		u32 StartBit, NumTiles;

		NumTiles = (u32)(LinuxIOInst->DevInst->NumCols * (LinuxIOInst->DevInst->NumRows - 1U));
		/* Loc is NULL, it suggests all tiles are requested */
		StartBit = _XAie_GetTileBitPosFromLoc(LinuxIOInst->DevInst,
				XAie_TileLoc(0, 1));
		_XAie_SetBitInBitmap(LinuxIOInst->DevInst->DevOps->TilesInUse, StartBit,NumTiles);
	} else {
		for(u32 i = 0; i < InitArgs.num_tiles; i++) {
			u32 Bit;

			if(InitArgs.locs[i].row == 0U) {
				continue;
			}

			/*
			 * If a tile is ungated, the rows below it are
			 * ungated.
			 */
			Bit = _XAie_GetTileBitPosFromLoc(LinuxIOInst->DevInst,
					XAie_TileLoc(InitArgs.locs[i].col, 1));
			_XAie_SetBitInBitmap(LinuxIOInst->DevInst->DevOps->TilesInUse,Bit, InitArgs.locs[i].row);
		}
	}

	Ret = ioctl(LinuxIOInst->PartitionFd, AIE_PARTITION_INIT_IOCTL,
			&InitArgs);
	free(InitArgs.locs);

	if (Ret != 0) {
		XAIE_ERROR("Failed to initialize partition, %d: %s\n",
			   errno, strerror(errno));
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API tears down the AI engine partition
*
* @param	IOInst: IO Instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal only.
*******************************************************************************/
AieRC _XAie_LinuxIO_TeardownPart(void *IOInst)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *) IOInst;
	int Ret;

	Ret = ioctl(LinuxIOInst->PartitionFd, AIE_PARTITION_TEAR_IOCTL);
	if (Ret != 0) {
		XAIE_ERROR("Failed to teardown partition, %d: %s\n",
			   errno, strerror(errno));
		return XAIE_ERR;
	}

	return XAIE_OK;
}
/*****************************************************************************/
/**

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
static AieRC XAie_LinuxIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC;

	switch(Op) {
	case XAIE_BACKEND_OP_CONFIG_SHIMDMABD:
		return _XAie_LinuxIO_ConfigShimDmaBd(IOInst, Arg);
	case XAIE_BACKEND_OP_UPDATE_SHIM_DMA_BD_ADDR:
		return _XAie_LinuxIO_UpdateShimDmaBdAddrOff(IOInst, Arg);
	case XAIE_BACKEND_OP_REQUEST_TILES:
		RC = _XAie_LinuxIO_RequestTiles(IOInst, Arg);
		if(RC == XAIE_OK)
			_XAie_IOCommon_MarkTilesInUse(DevInst,
					(XAie_BackendTilesArray *)Arg);
		return RC;
	case XAIE_BACKEND_OP_RELEASE_TILES:
		return _XAie_LinuxIO_ReleaseTiles(IOInst, Arg);
	case XAIE_BACKEND_OP_PARTITION_INITIALIZE:
		return _XAie_LinuxIO_InitPart(IOInst, (XAie_PartInitOpts *)Arg);
	case XAIE_BACKEND_OP_PARTITION_TEARDOWN:
		return _XAie_LinuxIO_TeardownPart(IOInst);
	case XAIE_BACKEND_OP_PARTITION_CLEAR_CONTEXT:
		return _XAie_LinuxIO_PartClearContext((XAie_LinuxIO *)IOInst);
	case XAIE_BACKEND_OP_SET_COLUMN_CLOCK:
		return _XAie_LinuxIO_SetColumnClock(IOInst, Arg);
	case XAIE_BACKEND_OP_PERFORMANCE_UTILIZATION:
		return _XAie_LinuxIO_PerfUtilization(IOInst, Arg);
	default:
		XAIE_ERROR("Linux backend does not support operation %d\n", Op);
		return XAIE_FEATURE_NOT_SUPPORTED;
	}
}

static u64 XAie_LinuxGetTid(void)
{
		return (u64)pthread_self();
}

static int XAie_LinuxGetPartFd(void *IOInst)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;

	return LinuxIOInst->PartitionFd;
}


/*****************************************************************************/
/**
*
* This is the IO function to submit a transaction to the kernel driver for
* execution.
*
* @param	IOInst: IO instance pointer
* @param	TxnInst: Pointer to the transaction instance.
*
* @return	XAIE_OK for success and error code for failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_LinuxSubmitTxn(void *IOInst, XAie_TxnInst *TxnInst)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	int Ret;
	struct aie_txn_inst Args;

	Args.num_cmds = TxnInst->NumCmds;
	Args.cmdsptr = (u64)TxnInst->CmdBuf;

	Ret = ioctl(LinuxIOInst->PartitionFd, AIE_TRANSACTION_IOCTL, &Args);
	if(Ret < 0) {
		XAIE_ERROR("Submitting transaction to device failed, %d: %s\n",
			errno, strerror(errno));
		return XAIE_ERR;
	}

	return XAIE_OK;
}

static XAie_MemInst* XAie_LinuxMemAllocate(XAie_DevInst* DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)DevInst->IOInst;
	XAie_MemInst *MemInst;
	XAie_LinuxMem *LinuxMemInst;
	void* Buf;
	int Fd;

	MemInst = (XAie_MemInst *)malloc(sizeof(XAie_MemInst));
	if(MemInst == NULL) {
		return NULL;
	}
	LinuxMemInst = (XAie_LinuxMem *) malloc(sizeof(XAie_LinuxMem));
	if (LinuxMemInst == NULL) {
		free(MemInst);
		return NULL;
	}

	Fd = ioctl(LinuxIOInst->PartitionFd, AIE_DMA_MEM_ALLOCATE_IOCTL,
		    &Size);
	if(Fd < 0) {
		XAIE_ERROR("Memory Allocation Failed");
		free(MemInst);
		free(LinuxMemInst);
		return NULL;
	}

	MemInst->Size = Size;
	MemInst->DevInst = DevInst;
	MemInst->Cache = Cache;

	Buf = mmap(NULL, MemInst->Size, PROT_READ | PROT_WRITE, MAP_SHARED,
		   Fd, 0);
	if(Buf == MAP_FAILED) {
		XAIE_ERROR("Memory Mapping Failed!: %d\n", errno);
		free(MemInst);
		free(LinuxMemInst);
		return NULL;
	}

	MemInst->VAddr = Buf;
	MemInst->DevAddr = 0x0;

	LinuxMemInst->BufferFd = Fd;
	MemInst->BackendHandle = (void *) LinuxMemInst;

	return MemInst;
}

static AieRC XAie_LinuxMemFree(XAie_MemInst *MemInst)
{
	XAie_LinuxMem* LinuxMemInst = (XAie_LinuxMem *)MemInst->BackendHandle;
	XAie_LinuxIO* LinuxIOInst = (XAie_LinuxIO *)MemInst->DevInst->IOInst;
	int Ret;

	Ret = munmap(MemInst->VAddr, MemInst->Size);
	if (Ret < 0) {
		free(MemInst->BackendHandle);
		free(MemInst);
		return XAIE_ERR;
	}

	Ret = ioctl(LinuxIOInst->PartitionFd, AIE_DMA_MEM_FREE_IOCTL,
		    &LinuxMemInst->BufferFd);
	if (Ret < 0) {
		free(MemInst->BackendHandle);
		free(MemInst);
		return XAIE_ERR;
	}

	close(LinuxMemInst->BufferFd);
	free(MemInst->BackendHandle);
	free(MemInst);
	MemInst = XAIE_NULL;

	return XAIE_OK;
}

static AieRC XAie_LinuxMemSyncForCPU(XAie_MemInst *MemInst)
{
	struct dma_buf_sync Sync;
	XAie_LinuxMem *LinuxMem;
	int Ret;

	LinuxMem = (XAie_LinuxMem *) MemInst->BackendHandle;
	if (LinuxMem == XAIE_NULL) {
		return XAIE_ERR;
	}

	Sync.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
	Ret = ioctl(LinuxMem->BufferFd, DMA_BUF_IOCTL_SYNC, &Sync);
	if(Ret < 0) {
		return XAIE_ERR;
	}

	return XAIE_OK;
}

static AieRC XAie_LinuxMemSyncForDev(XAie_MemInst *MemInst)
{
	struct dma_buf_sync Sync;
	XAie_LinuxMem *LinuxMem;
	int Ret;

	LinuxMem = (XAie_LinuxMem *) MemInst->BackendHandle;
	if (LinuxMem == XAIE_NULL) {
		return XAIE_ERR;
	}

	Sync.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;

	Ret = ioctl(LinuxMem->BufferFd, DMA_BUF_IOCTL_SYNC, &Sync);
	if(Ret < 0) {
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function submits an asynchronous 64-byte block write to the AIE device
* using io_uring. The write data is embedded directly in the SQE command buffer
* to avoid additional memory allocations.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to write to.
* @param	Data: Pointer to data buffer containing u32 words to write.
* @param	Size: Number of u32 words to write (max 16, i.e. 64 bytes).
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking.
*
* @return	Number of SQEs submitted on success, or 0 on failure. AsyncRes->res will contain
* 		the error code on failure.
*
* @note		Size must not exceed 16 u32 words (64 bytes).
*
*******************************************************************************/
int XAie_LinuxIO_Write64Bytes_Async(void *IOInst, u64 RegOff, const u32 *Data,
				    u32 Size, XAie_AsyncRes *AsyncRes)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	struct aie_block_write64 *Args;
	struct io_uring_sqe *Sqe;
	int Ret;

	AsyncRes->io_vec_inuse = 0;
	if (Size > 16) {
		XAIE_ERROR("Size exceeds 16-u32 (64 bytes) for async write\n");
		AsyncRes->res = -EINVAL;
		return 0;
	}
	Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
	if (!Sqe) {
		XAIE_ERROR("Failed to get sqe for async write\n");
		AsyncRes->res = -ENOMEM;
		return 0;
	}
	Sqe->opcode = IORING_OP_URING_CMD;
	Sqe->cmd_op = AIE_BLOCK_WRITE64_IOCTL;
	Sqe->flags |= IOSQE_FIXED_FILE;
	Sqe->addr = RegOff;
	Sqe->user_data = (u64)AsyncRes;
	Args = (struct aie_block_write64 *)Sqe->cmd;
	Args->size = Size;
	memcpy(Args->data, Data, Size * sizeof(u32));

	Ret = io_uring_submit(&LinuxIOInst->Ring);
	if (Ret < 0) {
		XAIE_ERROR("Failed to submit async write: %d\n", Ret);
		AsyncRes->res = Ret;
		return 0;
	}
	return 1;
}

/*****************************************************************************/
/**
*
* This function submits an asynchronous block write to the AIE device using
* io_uring. The write data is copied into a registered iovec buffer and
* submitted via the AIE_REG_BLOCKWRITE_CMD ioctl. PM and DM memory regions
* are not supported and will return an error.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to write to.
* @param	Data: Pointer to data buffer containing u32 words to write.
* @param	Size: Number of u32 words to write.
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking. On error, AsyncRes->res is set to the
*			corresponding error code.
*
* @return	Number of SQEs submitted on success, or 0 on failure.
*
* @note		Internal only. Does not support PM/DM memory writes.
*
*******************************************************************************/
static int XAie_LinuxIO_BlockWrite32Async(void *IOInst, u64 RegOff,
					  const u32 *Data, u32 Size,
					  XAie_AsyncRes *AsyncRes)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	struct aie_reg_args *Args;
	struct io_uring_sqe *Sqe;
	u32 *VirtAddr;
	u32 BufIdx;
	int Ret;

	/* PM and DM memory regions are not supported for async block write */
	VirtAddr = _XAie_GetVirtAddrFromOffset(LinuxIOInst, RegOff, Size);
	if (VirtAddr != NULL) {
		XAIE_ERROR("Async block write not supported for PM/DM memory\n");
		AsyncRes->res = -EINVAL;
		return 0;
	}
	if (VirtAddr == (u32 *)XAIE_INVALID_TILE) {
		XAIE_ERROR("Invalid tile for async block write\n");
		AsyncRes->res = -EINVAL;
		return 0;
	}

	if (_XAie_LinuxIO_IoVecIsFull(LinuxIOInst)) {
		XAIE_ERROR("io_uring iovec array is full, cannot submit async block write\n");
		AsyncRes->res = -ENOMEM;
		return 0;
	}

	if (Size * sizeof(u32) > (size_t)getpagesize()) {
		XAIE_ERROR("Block write size exceeds iovec buffer size\n");
		AsyncRes->res = -EINVAL;
		return 0;
	}

	BufIdx = _XAie_LinuxIO_IoVecIncHead(LinuxIOInst);
	memcpy(LinuxIOInst->IoVecs[BufIdx].iov_base, Data, Size * sizeof(u32));
	AsyncRes->io_vec_inuse = 1;

	Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
	if (Sqe == NULL) {
		XAIE_ERROR("Failed to get sqe for async block write\n");
		AsyncRes->res = -ENOMEM;
		_XAie_LinuxIO_IoVecDecHead(LinuxIOInst);
		return 0;
	}
	Sqe->opcode = IORING_OP_URING_CMD;
	Sqe->flags |= IOSQE_FIXED_FILE;
	Sqe->cmd_op = AIE_REG_BLOCKWRITE_CMD;
	Sqe->user_data = (u64)AsyncRes;
	Sqe->buf_index = BufIdx;
	Args = (struct aie_reg_args *)Sqe->cmd;
	*Args = (struct aie_reg_args){
		.op = AIE_REG_BLOCKWRITE,
		.offset = RegOff,
		.dataptr = (u64)LinuxIOInst->IoVecs[BufIdx].iov_base,
		.len = Size,
	};

	Ret = io_uring_submit(&LinuxIOInst->Ring);
	if (Ret < 0) {
		XAIE_ERROR("Failed to submit async block write: %d\n", Ret);
		AsyncRes->res = Ret;
		_XAie_LinuxIO_IoVecDecHead(LinuxIOInst);
		return 0;
	}

	return 1;
}

/*****************************************************************************/
/**
*
* This function submits an asynchronous block set to the AIE device using
* io_uring. It initializes a chunk of AIE address space with a specified
* value at 32-bit granularity. PM and DM memory regions are not supported
* and will return an error.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to write to.
* @param	Data: 32-bit value to fill the address range with.
* @param	Size: Number of 32-bit words to set.
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking. On error, AsyncRes->res is set to the
*			corresponding error code.
*
* @return	Number of SQEs submitted on success, or 0 on failure.
*
* @note		Internal only. Does not support PM/DM memory regions.
*
*******************************************************************************/
static int XAie_LinuxIO_BlockSet32Async(void *IOInst, u64 RegOff, u32 Data,
					u32 Size, XAie_AsyncRes *AsyncRes)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	struct aie_reg_args *Args;
	struct io_uring_sqe *Sqe;
	u32 *VirtAddr;
	int Ret;

	AsyncRes->io_vec_inuse = 0;

	/* PM and DM memory regions are not supported for async block set */
	VirtAddr = _XAie_GetVirtAddrFromOffset(LinuxIOInst, RegOff, Size);
	if (VirtAddr != NULL) {
		XAIE_ERROR("Async block set not supported for PM/DM memory\n");
		AsyncRes->res = -EINVAL;
		return 0;
	}
	if (VirtAddr == (u32 *)XAIE_INVALID_TILE) {
		XAIE_ERROR("Invalid tile for async block set\n");
		AsyncRes->res = -EINVAL;
		return 0;
	}

	Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
	if (Sqe == NULL) {
		XAIE_ERROR("Failed to get sqe for async block set\n");
		AsyncRes->res = -ENOMEM;
		return 0;
	}
	Sqe->opcode = IORING_OP_URING_CMD;
	Sqe->flags |= IOSQE_FIXED_FILE;
	Sqe->cmd_op = AIE_REG_BLOCKSET_CMD;
	Sqe->user_data = (u64)AsyncRes;
	Args = (struct aie_reg_args *)Sqe->cmd;
	*Args = (struct aie_reg_args){
		.offset = RegOff,
		.val = Data,
		.len = Size,
		.mask = 0,
	};

	Ret = io_uring_submit(&LinuxIOInst->Ring);
	if (Ret < 0) {
		XAIE_ERROR("Failed to submit async block set: %d\n", Ret);
		AsyncRes->res = Ret;
		return 0;
	}

	return 1;
}

static int XAie_LinuxIO_WriteBdAsync(void *IOInst, XAie_DmaDesc *DmaDesc,
				     XAie_LocType Loc, u16 BdNum, XAie_AsyncRes *AsyncRes)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)IOInst;
	XAie_DevInst *DevInst = LinuxIOInst->DevInst;
	struct io_uring_sqe *Sqe;
	const XAie_DmaMod *DmaMod = DmaDesc->DmaMod;
	u64 Addr;
	int ret;

	AsyncRes->io_vec_inuse = 0;
	if (DmaDesc->TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
		struct aie_dmabuf_bd_cmd * args;
		XAie_LinuxMem *LinuxMemInst;

		if (DmaDesc->MemInst == NULL) {
			XAIE_ERROR("DMA descriptor must have a valid memory instance for async BD write\n");
			AsyncRes->res = -EINVAL;
			return 0;
		}
		LinuxMemInst = (XAie_LinuxMem *) DmaDesc->MemInst->BackendHandle;

		if ((DmaMod->BdSize * sizeof(u32)) > sizeof(args->bd)) {
			XAIE_ERROR("BD size exceeds command buffer capacity for async BD write\n");
			AsyncRes->res = -EINVAL;
			return 0;
		}

		Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
		if (Sqe == NULL) {
			XAIE_ERROR("Failed to get sqe for async BD write\n");
			AsyncRes->res = -ENOMEM;
			return 0;
		}
		Sqe->opcode = IORING_OP_URING_CMD;
		Sqe->flags |= IOSQE_FIXED_FILE;
		Sqe->cmd_op = AIE_CONFIG_SHIMDMA_DMABUF_BD_CMD;
		Sqe->user_data = (u64)AsyncRes;
		Sqe->addr = BdNum;
		Sqe->buf_index = LinuxMemInst->BufferFd;
		args = (struct aie_dmabuf_bd_cmd *)Sqe->cmd;
		args->loc.col = Loc.Col;
		args->loc.row = Loc.Row;

		DmaMod->WriteBdPrep(DevInst, DmaDesc, Loc, BdNum, args->bd, &Addr);
		ret = io_uring_submit(&LinuxIOInst->Ring);
		if (ret < 0) {
			XAIE_ERROR("Failed to submit async BD write: %d\n", ret);
			AsyncRes->res = ret;
			return 0;
		}

		return 1;
	} else {
		struct aie_block_write64 *Args;

		if ((DmaMod->BdSize * sizeof(u32)) > sizeof(Args->data)) {
			XAIE_ERROR("BD size exceeds 64 bytes for async BD write\n");
			AsyncRes->res = -EINVAL;
			return 0;
		}
		Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
		if (Sqe == NULL) {
			XAIE_ERROR("Failed to get sqe for async BD write\n");
			AsyncRes->res = -ENOMEM;
			return 0;
		}
		Sqe->opcode = IORING_OP_URING_CMD;
		Sqe->flags |= IOSQE_FIXED_FILE;
		Sqe->cmd_op = AIE_BLOCK_WRITE64_IOCTL;
		Sqe->user_data = (u64)AsyncRes;
		Args = (struct aie_block_write64 *)Sqe->cmd;
		Args->size = DmaMod->BdSize * sizeof(u32);
		DmaMod->WriteBdPrep(DevInst, DmaDesc, Loc, BdNum, Args->data, &Addr);
		Sqe->addr = Addr;
		ret = io_uring_submit(&LinuxIOInst->Ring);
		if (ret < 0) {
			XAIE_ERROR("Failed to submit async BD write: %d\n", ret);
			AsyncRes->res = ret;
			return 0;
		}
		return 1;
	}
}

static int XAie_LinuxIO_UpdateShimDmaBdAddrOffAsync(XAie_MemInst *MemInst, XAie_LocType Loc,
						    u32 Offset, u16 BdNum, XAie_AsyncRes *AsyncRes)
{
	XAie_LinuxIO *LinuxIOInst = (XAie_LinuxIO *)MemInst->DevInst->IOInst;
	struct io_uring_sqe *Sqe;
	XAie_LinuxMem *LinuxMemInst;
	struct aie_dmabuf_bd_cmd *args;
	int ret;

	AsyncRes->io_vec_inuse = 0;
	if (MemInst == NULL) {
		XAIE_ERROR("Memory instance cannot be NULL for async BD address update\n");
		AsyncRes->res = -EINVAL;
		return 0;
	}
	LinuxMemInst = (XAie_LinuxMem *) MemInst->BackendHandle;

	Sqe = io_uring_get_sqe(&LinuxIOInst->Ring);
	if (Sqe == NULL) {
		XAIE_ERROR("Failed to get sqe for async BD address update\n");
		AsyncRes->res = -ENOMEM;
		return 0;
	}
	Sqe->opcode = IORING_OP_URING_CMD;
	Sqe->flags |= IOSQE_FIXED_FILE;
	Sqe->cmd_op = AIE_UPDATE_SHIMDMA_DMABUF_BD_ADDR_IOCTL;
	Sqe->user_data = (u64)AsyncRes;
	Sqe->addr = BdNum;
	Sqe->buf_index = LinuxMemInst->BufferFd;
	args = (struct aie_dmabuf_bd_cmd *)Sqe->cmd;
	args->loc.col = Loc.Col;
	args->loc.row = Loc.Row;
	args->bd[0] = Offset;

	ret = io_uring_submit(&LinuxIOInst->Ring);
	if (ret < 0) {
		XAIE_ERROR("Failed to submit async BD address update: %d\n", ret);
		AsyncRes->res = ret;
		return 0;
	}

	return 1;
}

#else /* __AIELINUX__ */

static AieRC XAie_LinuxIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

static AieRC XAie_LinuxIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with Linux kernel backend "
			"(__AIELINUX__)\n");
	return XAIE_INVALID_BACKEND;
}

static AieRC XAie_LinuxIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	return 0;
}

static AieRC XAie_LinuxIO_Write32(void *IOInst, u64 RegOff, u32 Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;

	return XAIE_ERR;
}

static AieRC XAie_LinuxIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Data;

	return XAIE_ERR;
}

static int XAie_LinuxIO_MaskWrite32_Async(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, XAie_AsyncRes *AsyncRes)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)AsyncRes;
	return 0;
}

static AieRC XAie_LinuxIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;
	return XAIE_ERR;
}

static AieRC XAie_LinuxIO_BlockWrite32(void *IOInst, u64 RegOff,
		const u32 *Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_LinuxIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data,
		u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_LinuxIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

static XAie_MemInst* XAie_LinuxMemAllocate(XAie_DevInst* DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void) DevInst;
	(void) Size;
	(void) Cache;

	return XAIE_NULL;
}

static AieRC XAie_LinuxMemFree(XAie_MemInst *MemInst)
{
	(void) MemInst;

	return XAIE_ERR;
}

static AieRC XAie_LinuxMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void) MemInst;

	return XAIE_ERR;
}

static AieRC XAie_LinuxMemSyncForDev(XAie_MemInst *MemInst)
{
	(void) MemInst;

	return XAIE_ERR;
}


static AieRC XAie_LinuxMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

static AieRC XAie_LinuxMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static u64 XAie_LinuxGetTid(void)
{
	return 0;
}

static int XAie_LinuxGetPartFd(void *IOInst)
{
	(void)IOInst;
	return 0;
}

static AieRC XAie_LinuxIO_GetPartitionList(XAie_DevInst *DevInst)
{
        (void)DevInst;
        return 0;
}

static AieRC XAie_LinuxSubmitTxn(void *IOInst, XAie_TxnInst *TxnInst)
{
	(void)IOInst;
	(void)TxnInst;
	return XAIE_ERR;
}

static void* XAie_LinuxIO_GetShimDmaBdConfig(XAie_ShimDmaBdArgs *Args)
{
	(void)Args;
	return XAIE_NULL;
}

static int XAie_LinuxIO_Write32_Async(void *IOInst, u64 RegOff, u32 Value,
				      XAie_AsyncRes *AsyncRes)
{
	(void)IOInst;
	(void)RegOff;
	(void)Value;
	(void)AsyncRes;
	return 0;
}

static AieRC XAie_LinuxIO_AsyncWaitNr(void *IOInst, u32 Nr)
{
	(void)IOInst;
	(void)Nr;
	return XAIE_ERR;
}

static AieRC XAie_LinuxIO_AsyncWait(void *IOInst)
{
	(void)IOInst;
	return XAIE_ERR;
}

static int XAie_LinuxMemAttachAsync(XAie_MemInst *MemInst, u64 MemHandle,
				    XAie_AsyncRes *AsyncRes)
{
	(void)MemInst;
	(void)MemHandle;
	(void)AsyncRes;
	return 0;
}

static int XAie_LinuxMemDetachAsync(XAie_MemInst *MemInst,
				    XAie_AsyncRes *AsyncRes)
{
	(void)MemInst;
	(void)AsyncRes;
	return 0;
}

static int XAie_LinuxIO_PartitionInitAsync(void *IOInst,
					   XAie_PartInitOpts *Opts,
					   XAie_AsyncRes *AsyncRes)
{
	(void)IOInst;
	(void)Opts;
	(void)AsyncRes;
	return 0;
}

static int XAie_LinuxIO_TeardownPartAsync(void *IOInst,
					  XAie_AsyncRes *AsyncRes)
{
	(void)IOInst;
	(void)AsyncRes;
	return 0;
}

static int XAie_LinuxIO_PartClearContextAsync(void *IOInst,
					      XAie_AsyncRes *AsyncRes)
{
	(void)IOInst;
	(void)AsyncRes;
	return 0;
}

static int XAie_LinuxIO_BlockWrite32Async(void *IOInst, u64 RegOff,
					  const u32 *Data, u32 Size,
					  XAie_AsyncRes *AsyncRes)
{
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
	(void)AsyncRes;
	return 0;
}

int XAie_LinuxIO_Write64Bytes_Async(void *IOInst, u64 RegOff, const u32 *Data,
				    u32 Size, XAie_AsyncRes *AsyncRes)
{
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
	(void)AsyncRes;
	return 0;
}

static int XAie_LinuxIO_BlockSet32Async(void *IOInst, u64 RegOff, u32 Data,
					u32 Size, XAie_AsyncRes *AsyncRes)
{
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;
	(void)AsyncRes;
	return 0;
}

static int XAie_LinuxIO_WriteBdAsync(void *IOInst, XAie_DmaDesc *DmaDesc,
				     XAie_LocType Loc, u16 BdNum,
				     XAie_AsyncRes *AsyncRes)
{
	(void)IOInst;
	(void)DmaDesc;
	(void)Loc;
	(void)BdNum;
	(void)AsyncRes;
	return 0;
}

static int XAie_LinuxIO_UpdateShimDmaBdAddrOffAsync(XAie_MemInst *MemInst,
						    XAie_LocType Loc,
						    u32 Offset, u16 BdNum,
						    XAie_AsyncRes *AsyncRes)
{
	(void)MemInst;
	(void)Loc;
	(void)Offset;
	(void)BdNum;
	(void)AsyncRes;
	return 0;
}

#endif /* __AIELINUX__ */

static AieRC XAie_LinuxIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
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

const XAie_Backend LinuxBackend =
{
	.Type = XAIE_IO_BACKEND_LINUX,
	.Ops.Init = XAie_LinuxIO_Init,
	.Ops.Finish = XAie_LinuxIO_Finish,
	.Ops.Write32 = XAie_LinuxIO_Write32,
	.Ops.Write32Async = XAie_LinuxIO_Write32_Async,
	.Ops.AsyncWait = XAie_LinuxIO_AsyncWait,
	.Ops.AsyncWaitNr = XAie_LinuxIO_AsyncWaitNr,
	.Ops.Read32 = XAie_LinuxIO_Read32,
	.Ops.MaskWrite32 = XAie_LinuxIO_MaskWrite32,
	.Ops.MaskWrite32Async = XAie_LinuxIO_MaskWrite32_Async,
	.Ops.MaskPoll = XAie_LinuxIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_LinuxIO_BlockWrite32,
	.Ops.BlockWrite32Async = XAie_LinuxIO_BlockWrite32Async,
	.Ops.BlockWrite64BytesAsync = XAie_LinuxIO_Write64Bytes_Async,
	.Ops.BlockSet32 = XAie_LinuxIO_BlockSet32,
	.Ops.BlockSet32Async = XAie_LinuxIO_BlockSet32Async,
	.Ops.CmdWrite = XAie_LinuxIO_CmdWrite,
	.Ops.RunOp = XAie_LinuxIO_RunOp,
	.Ops.MemAllocate = XAie_LinuxMemAllocate,
	.Ops.MemFree = XAie_LinuxMemFree,
	.Ops.MemSyncForCPU = XAie_LinuxMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_LinuxMemSyncForDev,
	.Ops.MemAttach = XAie_LinuxMemAttach,
	.Ops.MemAttachAsync = XAie_LinuxMemAttachAsync,
	.Ops.MemDetach = XAie_LinuxMemDetach,
	.Ops.MemDetachAsync = XAie_LinuxMemDetachAsync,
	.Ops.GetTid = XAie_LinuxGetTid,
	.Ops.GetPartFd = XAie_LinuxGetPartFd,
	.Ops.SubmitTxn = XAie_LinuxSubmitTxn,
	.Ops.GetShimDmaBdConfig = XAie_LinuxIO_GetShimDmaBdConfig,
	.Ops.GetPartitionList = XAie_LinuxIO_GetPartitionList,
	.Ops.SetPadInteger = NULL,
	.Ops.SetPadString = NULL,
	.Ops.PartitionInitAsync = XAie_LinuxIO_PartitionInitAsync,
	.Ops.PartitionTeardownAsync = XAie_LinuxIO_TeardownPartAsync,
	.Ops.PartClearContextAsync = XAie_LinuxIO_PartClearContextAsync,
	.Ops.WriteBdAsync = XAie_LinuxIO_WriteBdAsync,
	.Ops.UpdateShimDmaBdAddrOffAsync = XAie_LinuxIO_UpdateShimDmaBdAddrOffAsync,
};

/** @} */
