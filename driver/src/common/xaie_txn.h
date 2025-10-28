/******************************************************************************
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_txn.h
* @{
*
* This file contains data structure for TxN flow
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Keyur   08/25/2023  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIETXN_H
#define XAIETXN_H
#include <limits.h>

/***************************** Include Files *********************************/
/* All New custom Ops should be added above XAIE_IO_CUSTOM_OP_NEXT
 * To support backward compatibility existing enums should not be
 * modified. */
typedef enum {
	XAIE_IO_WRITE,
	XAIE_IO_BLOCKWRITE,
	XAIE_IO_BLOCKSET,
	XAIE_IO_MASKWRITE,
	XAIE_IO_MASKPOLL,
	XAIE_IO_NOOP,
	XAIE_IO_PREEMPT,
	XAIE_IO_MASKPOLL_BUSY,
	XAIE_IO_LOADPDI,
	XAIE_IO_LOAD_PM_START,
	XAIE_IO_CREATE_SCRATCHPAD,
	XAIE_IO_UPDATE_STATE_TABLE,
	XAIE_IO_UPDATE_REG,
	XAIE_IO_UPDATE_SCRATCH,
	XAIE_CONFIG_SHIMDMA_BD,
	XAIE_CONFIG_SHIMDMA_DMABUF_BD,
	XAIE_IO_CUSTOM_OP_BEGIN = 1U<<7U,
	XAIE_IO_CUSTOM_OP_TCT = XAIE_IO_CUSTOM_OP_BEGIN,
	XAIE_IO_CUSTOM_OP_DDR_PATCH, // Previously this was XAIE_IO_CUSTOM_OP_BEGIN + 1
	XAIE_IO_CUSTOM_OP_READ_REGS, // Previously this was XAIE_IO_CUSTOM_OP_BEGIN + 2
	XAIE_IO_CUSTOM_OP_RECORD_TIMER, // Previously this was XAIE_IO_CUSTOM_OP_BEGIN + 3
	XAIE_IO_CUSTOM_OP_MERGE_SYNC, // Previously this was XAIE_IO_CUSTOM_OP_BEGIN + 4
	XAIE_IO_CUSTOM_OP_NEXT,
	XAIE_IO_LOAD_PM_END_INTERNAL = 200,
	XAIE_IO_CUSTOM_OP_MAX = UCHAR_MAX,
} XAie_TxnOpcode;

struct XAie_TxnCmd {
	XAie_TxnOpcode Opcode;
	u32 Mask;
	u64 RegOff;
	u32 Value;
	u32 Size;
	u64 DataPtr;
	u32 PmId;
	u16 PdiId;
	u8 Preempt_level;
	u64 ScratchOffset;
	u8 UsageType;
	u8 StateTableIdx;
	u32 Func;
	u32 FuncArg;
};

/* typedef to capture transaction buffer data */
typedef struct {
	u64 Tid;
	u32 Flags;
	u32 NumCmds;	// Actual no of command available in TXN Cmd Array
	u32 MaxCmds;	// Current allocated size of TXN Cmd Array
	u32 InitCmds;	// Initial allocated size of TXN Cmd Array
	u8  NextCustomOp;
	XAie_TxnCmd *CmdBuf;
	XAie_List Node;
} XAie_TxnInst;

typedef struct {
	uint8_t Major;
	uint8_t Minor;
	uint8_t DevGen;
	uint8_t NumRows;
	uint8_t NumCols;
	uint8_t NumMemTileRows;
	uint16_t padding;
	uint32_t NumOps;
	uint32_t TxnSize;
} XAie_TxnHeader;

typedef struct {
	uint8_t Op;
	uint8_t Col;
	uint8_t Row;
} XAie_OpHdr;

typedef struct {
	XAie_OpHdr OpHdr;
	uint64_t RegOff;
	uint32_t Value;
	uint32_t Size;
} XAie_Write32Hdr;

typedef struct {
	XAie_OpHdr OpHdr;
	uint64_t RegOff;
	uint32_t Value;
	uint32_t Mask;
	uint32_t Size;
} XAie_MaskWrite32Hdr;

typedef struct {
	XAie_OpHdr OpHdr;
	uint64_t RegOff;
	uint32_t Value;
	uint32_t Mask;
	uint32_t Size;
} XAie_MaskPoll32Hdr;

typedef struct {
	XAie_OpHdr OpHdr;
	uint8_t Col;
	uint8_t Row;
	uint32_t RegOff;
	uint32_t Size;
} XAie_BlockWrite32Hdr;

typedef struct {
	XAie_OpHdr OpHdr;
	uint32_t Size;
} XAie_CustomOpHdr;

typedef struct {
	uint8_t Op;
	uint8_t padding[3];
} XAie_NoOpHdr;

typedef enum {
	NOOP,
	MEM_TILE,
	AIE_TILE,
	AIE_REGISTERS,
	INVALID
} XAie_Preempt_level;

typedef struct {
	uint8_t Op;
	uint8_t Preempt_level;
	uint16_t Reserved;
} XAie_PreemptHdr;

typedef struct {
	uint8_t Op;
	uint8_t Padding;
	uint16_t PdiId;
	uint32_t PdiSize;
	uint64_t PdiAddress;
} XAie_LoadPdiHdr;

typedef struct {
	uint8_t Op;
	uint8_t LoadSequenceCount[3];
	uint32_t PmLoadId;
}XAie_PmLoadHdr;

typedef enum {
	XAIE_STATE_TABLE
} XAie_UsageType;

typedef struct {
	uint8_t Op;
	uint8_t UsageType;
	uint16_t padding;
	uint32_t Size;
	uint64_t ScratchOffset;
} XAie_CreateScratchpadHdr;

typedef enum {
	XAIE_STATE_TABLE_MUL,
	XAIE_STATE_TABLE_INCR,
	XAIE_STATE_TABLE_DECR,
} XAie_StateTableFuncType;

typedef struct {
	uint8_t Op;
	uint8_t StateTableIdx;
	uint8_t Func;
	uint8_t Reserved;
	uint32_t FuncArg;
} XAie_UpdateStateHdr;

typedef struct {
	uint8_t Op;
	uint8_t StateTableIdx;
	uint8_t Func;
	uint8_t Reserved;
	uint32_t FuncArg;
	uint32_t RegOff;
} XAie_UpdateRegHdr;

typedef struct {
	uint8_t Op;
	uint8_t padding[3];
} XAie_UpdateScratchHdr;

/* Migrated from Aie-controller */

enum op_types {
    OP_LIST(GENERATE_ENUM)
};

typedef struct{
    enum op_types type;
    unsigned int size_in_bytes;
} op_base;

typedef struct {
    uint32_t word;
    uint32_t config;
} tct_op_t;

typedef struct {
    op_base b;
    u32 action;
    u64 regaddr; // regaddr to patch
    u64 argidx;  // kernel arg idx to get value to write at regaddr
    u64 argplus; // value to add to what's passed @ argidx (e.g., offset to shim addr)
} patch_op_t;

/*
 * Structs for reading registers
 */
typedef struct {
    uint64_t address;
} register_data_t;

typedef struct {
    uint32_t count;
    register_data_t data[1]; // variable size
} read_register_op_t;

/*
 * Struct for record timer identifier
 */
typedef struct {
    uint32_t id;
} record_timer_op_t;



/* Optimized Txn structs start from here */
typedef struct{
	uint8_t Op;
	uint8_t padding[3];
} XAie_OpHdr_opt;

typedef struct {
	XAie_OpHdr_opt OpHdr;
	uint32_t RegOff;
	uint32_t Value;
} XAie_Write32Hdr_opt;

typedef struct {
	XAie_OpHdr_opt OpHdr;
	uint32_t RegOff;
	uint32_t Value;
	uint32_t Mask;
} XAie_MaskWrite32Hdr_opt;

typedef struct {
	XAie_OpHdr_opt OpHdr;
	uint32_t RegOff;
	uint32_t Value;
	uint32_t Mask;
} XAie_MaskPoll32Hdr_opt;

typedef struct {
	XAie_OpHdr_opt OpHdr;
	uint32_t RegOff;
	uint32_t Size;
} XAie_BlockWrite32Hdr_opt;

typedef struct {
	XAie_OpHdr_opt OpHdr;
	uint32_t Size;
} XAie_CustomOpHdr_opt;

typedef struct {
    uint32_t regaddr; // regaddr to patch
    uint8_t argidx;  // kernel arg idx to get value to write at regaddr
    uint8_t padding[3];
    uint64_t argplus; // value to add to what's passed @ argidx (e.g., offset to shim addr)
} patch_op_opt_t;

typedef struct {
    uint32_t word;
    uint32_t config;
} tct_op_opt_t;

/*
 * Structs for reading registers
 */

typedef struct {
    uint64_t address;
} register_data_opt_t;

typedef struct {
    uint32_t count;
    uint32_t padding;
    register_data_opt_t data[1]; // variable size
} read_register_op_opt_t;

/*
 * Struct for record timer identifier
 */
typedef struct {
    uint32_t id;
} record_timer_op_opt_t;

XAIE_AIG_EXPORT AieRC XAie_CfgInitialTxnCmdArraySize(XAie_DevInst *DevInst, u32 CmdCount);
XAIE_AIG_EXPORT AieRC XAie_StartTransaction(XAie_DevInst *DevInst, u32 Flags);
XAIE_AIG_EXPORT AieRC XAie_SubmitTransaction(XAie_DevInst *DevInst, XAie_TxnInst *TxnInst);
XAie_TxnInst* XAie_ExportTransactionInstance(XAie_DevInst *DevInst);
XAIE_AIG_EXPORT u8* XAie_ExportSerializedTransaction(XAie_DevInst *DevInst,
		u8 NumConsumers, u32 Flags);
XAIE_AIG_EXPORT u8* XAie_ExportSerializedTransaction_opt(XAie_DevInst *DevInst,
		u8 NumConsumers, u32 Flags);
XAIE_AIG_EXPORT AieRC XAie_FreeTransactionInstance(XAie_TxnInst *TxnInst);
XAIE_AIG_EXPORT AieRC XAie_ClearTransaction(XAie_DevInst* DevInst);

XAIE_AIG_EXPORT int XAie_RequestCustomTxnOp(XAie_DevInst *DevInst);
XAIE_AIG_EXPORT AieRC XAie_AddCustomTxnOp(XAie_DevInst *DevInst, u8 OpNumber, void* Args, size_t size);
XAIE_AIG_EXPORT AieRC XAie_Txn_Write32(XAie_DevInst *DevInst, u64 RegOff, u32 Value);
XAIE_AIG_EXPORT AieRC XAie_Txn_Read32(XAie_DevInst *DevInst, u64 RegOff, u32 *Data);
XAIE_AIG_EXPORT AieRC XAie_Txn_MaskWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value);
XAIE_AIG_EXPORT AieRC XAie_Txn_MaskPoll(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value, u32 TimeOutUs);
XAIE_AIG_EXPORT AieRC XAie_Txn_MaskPollBusy(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value, u32 TimeOutUs);
XAIE_AIG_EXPORT AieRC XAie_Txn_BlockWrite32(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data, u32 Size);
XAIE_AIG_EXPORT AieRC XAie_Txn_BlockSet32(XAie_DevInst *DevInst, u64 RegOff, const u32 Data, u32 Size);
XAIE_AIG_EXPORT AieRC XAie_Txn_NoOp(XAie_DevInst *DevInst, uint32_t Count);
XAIE_AIG_EXPORT AieRC XAie_Txn_Preempt(XAie_DevInst *DevInst, XAie_PreemptHdr* Preempt);
XAIE_AIG_EXPORT AieRC XAie_Txn_LoadPdi(XAie_DevInst *DevInst, u16 PdiId);
XAIE_AIG_EXPORT AieRC XAie_Txn_PmLoadStart(XAie_DevInst *DevInst, u32 PmLoadId);
XAIE_AIG_EXPORT AieRC XAie_Txn_PmLoadEnd(XAie_DevInst *DevInst);
XAIE_AIG_EXPORT AieRC XAie_Txn_UpdateStateTable(XAie_DevInst *DevInst, u8 StateTableIdx, XAie_StateTableFuncType Func, u32 FuncArg);
XAIE_AIG_EXPORT AieRC XAie_Txn_UpdateReg(XAie_DevInst *DevInst, u64 RegOff, u8 StateTableIdx, XAie_StateTableFuncType Func, u32 FuncArg);
XAIE_AIG_EXPORT AieRC XAie_Txn_CreateScratchpad(XAie_DevInst *DevInst, u8 UsageType, u32 Size, u64 ScratchOffset);
XAIE_AIG_EXPORT AieRC XAie_Txn_UpdateScratch(XAie_DevInst *DevInst);
XAIE_AIG_EXPORT AieRC XAie_Txn_DdrAddressPatch(XAie_DevInst *DevInst, u64 regaddr, u64 argidx, u64 argplus);
XAIE_AIG_EXPORT AieRC XAie_Txn_MergeSync(XAie_DevInst *DevInst, u8 num_tokens, u8 num_cols);

XAIE_AIG_EXPORT AieRC XAie_Txn_RunOp(XAie_DevInst *DevInst, u8 Is_Config_ShimDMABD, void *Arg);

#endif
