/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_helper.h
* @{
*
* This file contains inline helper functions for AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   12/09/2019  Include correct header file to avoid cyclic
*			    dependancy
* 1.2   Tejus   03/22/2020  Remove helper functions used by initial dma
*			    implementations
* 1.3   Tejus   04/13/2020  Add api to get tile type from Loc
* 1.4   Tejus   04/13/2020  Remove helper functions for range apis
* 1.5   Tejus   06/10/2020  Add helper functions for IO backend.
* 1.6   Nishad  07/06/2020  Add helper functions for stream switch module.
* 1.7   Nishad  07/24/2020  Add _XAie_GetFatalGroupErrors() helper function.
* </pre>
*
******************************************************************************/
#ifndef XAIEHELPER_H
#define XAIEHELPER_H

/***************************** Include Files *********************************/
#include <limits.h>
#include "xaie_io.h"
#include "xaiegbl_regdef.h"
#include "xaie_core.h"
#include "xaie_dma.h"
#include "xaie_locks.h"

/***************************** Macro Definitions *****************************/
#define CheckBit(bitmap, pos)   ((bitmap)[(pos) / (sizeof((bitmap)[0]) * 8U)] & \
				 (1U << (pos) % (sizeof((bitmap)[0]) * 8U)))

#define XAIE_ERROR(...)							      \
	do {								      \
		XAie_Log(stderr, "[AIE ERROR]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

#define XAIE_WARN(...)							      \
	do {								      \
		XAie_Log(stderr, "[AIE WARNING]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

#ifdef XAIE_DEBUG

#define XAIE_DBG(...)							      \
	do {								      \
		XAie_Log(stdout, "[AIE DEBUG]", __func__, __LINE__,	      \
				__VA_ARGS__);				      \
	} while(0)

#else

#define XAIE_DBG(DevInst, ...) {}

#endif /* XAIE_DEBUG */

/* Compute offset of field within a structure */
#define XAIE_OFFSET_OF(structure, member) \
	((uintptr_t)&(((structure *)0)->member))

/* Compute a pointer to a structure given a pointer to one of its fields */
#define XAIE_CONTAINER_OF(ptr, structure, member) \
	(void*)((uintptr_t)(ptr) - XAIE_OFFSET_OF(structure, member))

/* Loop through the set bits in Value */
#define for_each_set_bit(Index, Value, Len)				      \
	for((Index) = first_set_bit((Value)) - 1;			      \
	    (Index) < (Len);						      \
	    (Value) &= (Value) - 1, (Index) = first_set_bit((Value)) - 1)

/* Generate value with a set bit at given Index */
#define BIT(Index)		(1 << (Index))

/*
 * __attribute is not supported for windows. remove it conditionally.
 */
#ifdef _MSC_VER
#define XAIE_PACK_ATTRIBUTE
#else
#define XAIE_PACK_ATTRIBUTE  __attribute__((packed, aligned(4)))
#endif
/**************************** Type Definitions *******************************/
typedef enum {
	XAIE_IO_WRITE,
	XAIE_IO_BLOCKWRITE,
	XAIE_IO_BLOCKSET,
	XAIE_IO_MASKWRITE,
	XAIE_IO_MASKPOLL,
	XAIE_CONFIG_SHIMDMA_BD,
	XAIE_CONFIG_SHIMDMA_DMABUF_BD,
	XAIE_IO_CUSTOM_OP_BEGIN = 1<<7,
	XAIE_IO_CUSTOM_OP_MAX = UCHAR_MAX,
} XAie_TxnOpcode;

struct XAie_TxnCmd {
	XAie_TxnOpcode Opcode;
	u32 Mask;
	u64 RegOff;
	u32 Value;
	u64 DataPtr;
	u32 Size;
};

/* Data structure to capture the dma status */
typedef struct {
	u32 S2MMStatus;
	u32 MM2SStatus;
} XAie_DmaStatus;

/* Data structure to capture the core tile status */
typedef struct {
	XAie_DmaStatus *Dma;
	u32 *EventCoreModStatus;
	u32 *EventMemModStatus;
	u32 CoreStatus;
	u32 ProgramCounter;
	u32 StackPtr;
	u32 LinkReg;
	u8  *LockValue;
} XAie_CoreTileStatus;

/* Data structure to capture the mem tile status */
typedef struct {
	XAie_DmaStatus *Dma;
	u32 *EventStatus;
	u8 *LockValue;
} XAie_MemTileStatus;

/* Data structure to capture the shim tile status */
typedef struct {
	XAie_DmaStatus *Dma;
	u32 *EventStatus;
	u8 *LockValue;
} XAie_ShimTileStatus;

/* Data structure to capture column status */
typedef struct {
	XAie_CoreTileStatus *CoreTile;
	XAie_MemTileStatus *MemTile;
	XAie_ShimTileStatus *ShimTile;
} XAie_ColStatus;

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* Calculates the Tile Address from Row, Col of the AIE array/partition
*
* @param	DevInst: Device Instance
* @param	R: Row
* @param	C: Column
* @return	TileAddr
*
* @note		Internal API only.
*
******************************************************************************/
static inline u64 _XAie_GetTileAddr(XAie_DevInst *DevInst, u8 R, u8 C)
{
	return (((u64)R & 0xFF) << DevInst->DevProp.RowShift) |
		(((u64)C & 0xFF) << DevInst->DevProp.ColShift);
}

/*****************************************************************************/
/**
*
* Calculates the index value of first set bit. Indexing starts with a value of
* 1.
*
* @param	Value: Value
* @return	Index of first set bit.
*
* @note		Internal API only.
*
******************************************************************************/
static inline u32 first_set_bit(u64 Value)
{
	u32 Index = 1;

	if (Value == 0)
		return 0;

	while (!(Value & 1)) {
		Value >>= 1;
		Index++;
	}

	return Index;
}

void XAie_Log(FILE *Fd, const char *prefix, const char *func, u32 line,
		const char *Format, ...);
u8 _XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC _XAie_CheckModule(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
AieRC _XAie_GetSlaveIdx(const XAie_StrmMod *StrmMod, StrmSwPortType Slave,
		u8 PortNum, u8 *SlaveIdx);
AieRC _XAie_GetMstrIdx(const XAie_StrmMod *StrmMod, StrmSwPortType Master,
		u8 PortNum, u8 *MasterIdx);
u32 _XAie_GetFatalGroupErrors(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
u32 _XAie_GetTileBitPosFromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
void _XAie_SetBitInBitmap(u32 *Bitmap, u32 StartSetBit, u32 NumSetBit);
void _XAie_ClrBitInBitmap(u32 *Bitmap, u32 StartSetBit, u32 NumSetBit);
AieRC XAie_Write32(XAie_DevInst *DevInst, u64 RegOff, u32 Value);
AieRC XAie_Read32(XAie_DevInst *DevInst, u64 RegOff, u32 *Data);
AieRC XAie_MaskWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value);
AieRC XAie_MaskPoll(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs);
AieRC XAie_BlockWrite32(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data,
			u32 Size);
AieRC XAie_BlockSet32(XAie_DevInst *DevInst, u64 RegOff, u32 Data, u32 Size);
AieRC XAie_CmdWrite(XAie_DevInst *DevInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr);
int BuffHexDump(char* buff,size_t size);
int XAie_RequestCustomTxnOp(XAie_DevInst *DevInst);
AieRC XAie_AddCustomTxnOp(XAie_DevInst *DevInst, u8 OpNumber, void* Args, size_t size);
AieRC XAie_RunOp(XAie_DevInst *DevInst, XAie_BackendOpCode Op, void *Arg);
AieRC _XAie_Txn_Start(XAie_DevInst *DevInst, u32 Flags);
AieRC _XAie_Txn_Submit(XAie_DevInst *DevInst, XAie_TxnInst *TxnInst);
XAie_TxnInst* _XAie_TxnExport(XAie_DevInst *DevInst);
u8* _XAie_TxnExportSerialized(XAie_DevInst *DevInst, u8 NumConsumers,
		u32 Flags);
AieRC _XAie_ClearTransaction(XAie_DevInst* DevInst);
AieRC _XAie_TxnFree(XAie_TxnInst *Inst);
void _XAie_TxnResourceCleanup(XAie_DevInst *DevInst);
u32 _XAie_GetNumRows(XAie_DevInst *DevInst, u8 TileType);
u32 _XAie_GetStartRow(XAie_DevInst *DevInst, u8 TileType);
void _XAie_FreeTxnPtr(void *Ptr);
AieRC XAie_StatusDump(XAie_DevInst *DevInst, XAie_ColStatus *Status);

#endif		/* end of protection macro */
/** @} */
