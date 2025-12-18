/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_txn.c
* @{
*
* This file contains transaction binary functions for AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Adhipathi   01/07/2025  Initial creation
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include "xaie_helper.h"
#include "xaie_txn.h"

// Major & Minor version details for TXN version 0.1 called Initial version
#define XAIE_TXN_VER_01_MAJOR 0
#define XAIE_TXN_VER_01_MINOR 1

// Major & Minor version details for TXN version 1.0 called optimized version
#define XAIE_TXN_VER_10_MAJOR 1
#define XAIE_TXN_VER_10_MINOR 0


/***************************** Macro Definitions *****************************/
#define XAIE_DEFAULT_NUM_CMDS 1024U
#define XAIE_DEFAULT_TXN_BUFFER_SIZE (1024 * 4)
#define XAIE_TXN_INSTANCE_EXPORTED	0b10U
#define XAIE_TXN_INST_EXPORTED_MASK XAIE_TXN_INSTANCE_EXPORTED
#define XAIE_TXN_AUTO_FLUSH_MASK XAIE_TRANSACTION_ENABLE_AUTO_FLUSH
#define XAIE_TXN_STATE_TABLE_SIZE 32U
#define TX_DUMP_ENABLE 0
#define XAIE_TXN_16KB_BOUNDARY (16U * 1024U)

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API creates transaction binary header using device info.
*
* @param        DevInst: Device Instance
* @param        Header: Pointer to the transaction header to be created
*
* @return       None
*
* @note         Internal only.
*
******************************************************************************/
static inline void _XAie_CreateTxnHeader(XAie_DevInst *DevInst,
		XAie_TxnHeader *Header)
{
	Header->Major = XAIE_TXN_VER_01_MAJOR;
	Header->Minor = XAIE_TXN_VER_01_MINOR;
	Header->DevGen = DevInst->DevProp.DevGen;
	Header->NumRows = DevInst->NumRows;
	Header->NumCols = DevInst->NumCols;
	Header->NumMemTileRows = DevInst->MemTileNumRows;
	XAIE_DBG("Header version %d.%d\n", Header->Major, Header->Minor);
	XAIE_DBG("Device Generation: %d\n", Header->DevGen);
	XAIE_DBG("Cols, Rows, MemTile rows : (%d, %d, %d)\n", Header->NumCols,
			Header->NumRows, Header->NumMemTileRows);
}

static inline u8 _XAie_GetRowfromRegOff(XAie_DevInst *DevInst, u64 RegOff)
{
	return RegOff &(u64)(~(ULONG_MAX << DevInst->DevProp.RowShift));
}

static inline u8 _XAie_GetColfromRegOff(XAie_DevInst *DevInst, u64 RegOff)
{
	u64 Mask = (u64)(((1U << DevInst->DevProp.ColShift) - 1U) &
			~((1U << DevInst->DevProp.RowShift) - 1U));

	return (u8)((RegOff & Mask) >> (u64)(DevInst->DevProp.RowShift));
}

/*****************************************************************************/
/**
* This API return the default TXN initial command array size.
*
* @param        None
*
* @return       Default TXN command array size (XAIE_DEFAULT_NUM_CMDS)
*
******************************************************************************/
u32 _XAie_Txn_GetDefTxnInitialCmdArraySize(void)
{
	return XAIE_DEFAULT_NUM_CMDS;
}

/*****************************************************************************/
/**
* This API sets the TXN Initial command array size to application layer
* configured value.
*
* @param        DevInst: Device Instance
* @param        CmdCount: Command count to be set.
*
* @return       XAIE_OK on success, error code on failure.
*
******************************************************************************/
AieRC _XAie_Txn_CfgInitialCmdArraySize(XAie_DevInst *DevInst, u32 CmdCount)
{
	if (CmdCount == 0) {
		return XAIE_INVALID_ARGS;
	}

	DevInst->InitialTxnCmdArraySize = CmdCount;
	XAIE_DBG(" Configured initial TXN command array size: %d\n", CmdCount);

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API inserts a transaction node to the linked list.
*
* @param        DevInst: Device Instance
* @param        TxnNode: Pointer to the transaction node to be inserted
*
* @return       None
*
* @note         Internal only.
*
******************************************************************************/
static void _XAie_AppendTxnInstToList(XAie_DevInst *DevInst, XAie_TxnInst *Inst)
{
	XAie_List *Node = &DevInst->TxnList;

	while(Node->Next != NULL) {
		Node = Node->Next;
	}

	Node->Next = &Inst->Node;
	Inst->Node.Next = NULL;
}

/*****************************************************************************/
/**
* This API returns the transaction list from the linked list based on the thread
* id.
*
* @param        DevInst: Device instance pointer
* @param	Tid: Thread id.
*
* @return       Pointer to transaction instance on success and NULL on failure
*
* @note         Internal only.
*
******************************************************************************/
static XAie_TxnInst *_XAie_GetTxnInst(XAie_DevInst *DevInst, u64 Tid)
{
	XAie_List *NodePtr = DevInst->TxnList.Next;
	XAie_TxnInst *TxnInst;

	while(NodePtr != NULL) {
		TxnInst = (XAie_TxnInst *)(uintptr_t)XAIE_CONTAINER_OF(NodePtr, XAie_TxnInst, Node);
		if(TxnInst->Tid == Tid) {
			return TxnInst;
		}

		NodePtr = NodePtr->Next;
	}

	return NULL;
}

/*****************************************************************************/
/**
* This API removes a node from the linked list if the thread id is found.
*
* @param        DevInst: Device instance pointer
* @param	Tid: Thread id.
*
* @return       XAIE_OK on success and error code on failure.
*
* @note         Internal only.
*
******************************************************************************/
static AieRC _XAie_RemoveTxnInstFromList(XAie_DevInst *DevInst, u64 Tid)
{
	XAie_List *NodePtr = DevInst->TxnList.Next;
	XAie_List *Prev = &DevInst->TxnList;
	XAie_TxnInst *Inst;

	while(NodePtr != NULL) {
		Inst = (XAie_TxnInst *)(uintptr_t)XAIE_CONTAINER_OF(NodePtr, XAie_TxnInst,
				Node);
		if(Inst->Tid == Tid) {
			break;
		}

		Prev = NodePtr;
		NodePtr = NodePtr->Next;
	}

	if(NodePtr == NULL) {
		XAIE_ERROR("Cannot find node to delete from list\n");
		return XAIE_ERR;
	} else {
		Prev->Next = NodePtr->Next;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This api allocates the memory to store the IO transaction commands
* when the driver is configured to execute in transaction mode.
*
* @param	DevInst - Device instance pointer.
* @param	Flags - Flags passed by the user.
*
* @return	Pointer to transaction instance on success and NULL on error.
*
* @note		Internal Only.
*
******************************************************************************/
AieRC _XAie_Txn_Start(XAie_DevInst *DevInst, u32 Flags)
{
	XAie_TxnInst *Inst;
	const XAie_Backend *Backend = DevInst->Backend;

	Inst = (XAie_TxnInst*)malloc(sizeof(*Inst));
	if(Inst == NULL) {
		XAIE_ERROR("Failed to allocate memory for txn instance\n");
		return XAIE_ERR;
	}

	Inst->CmdBuf = (XAie_TxnCmd*)calloc(DevInst->InitialTxnCmdArraySize,
			sizeof(*Inst->CmdBuf));
	if(Inst->CmdBuf == NULL) {
		XAIE_ERROR("Failed to allocate memory for command buffer\n");
		free(Inst);
		return XAIE_ERR;
	}
	XAIE_DBG(" Allocated initial TXN command array of size: %d\n", DevInst->InitialTxnCmdArraySize);

	Inst->NumCmds = 0U;
	Inst->MaxCmds = DevInst->InitialTxnCmdArraySize;
	Inst->InitCmds = DevInst->InitialTxnCmdArraySize;
	Inst->Tid = Backend->Ops.GetTid();
	Inst->NextCustomOp = (u8)XAIE_IO_CUSTOM_OP_NEXT;

	XAIE_DBG("Transaction buffer allocated with id: %ld\n", Inst->Tid);
	Inst->Flags = Flags;
	if(Flags & XAIE_TXN_AUTO_FLUSH_MASK) {
		XAIE_DBG("Auto flush is enabled for transaction buffer with "
				"id: %ld\n", Inst->Tid);
	} else {
		XAIE_DBG("Auto flush is disabled for transaction buffer with "
				"id: %ld\n", Inst->Tid);
	}

	_XAie_AppendTxnInstToList(DevInst, Inst);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This api copies an existing transaction instance and returns a copy of the
* instance with all the commands for users to save the commands and use them
* at a later point.
*
* @param	DevInst - Device instance pointer.
*
* @return	Pointer to copy of transaction instance on success and NULL
*		on error.
*
* @note		Internal only.
*
******************************************************************************/
XAie_TxnInst* _XAie_TxnExport(XAie_DevInst *DevInst)
{
	XAie_TxnInst *Inst, *TmpInst;
	const XAie_Backend *Backend = DevInst->Backend;

	TmpInst = _XAie_GetTxnInst(DevInst, Backend->Ops.GetTid());
	if(TmpInst == NULL) {
		XAIE_ERROR("Failed to get the correct transaction instance "
				"from internal list\n");
		return NULL;
	}

	Inst = (XAie_TxnInst *)malloc(sizeof(*Inst));
	if(Inst == NULL) {
		XAIE_ERROR("Failed to allocate memory for txn instance\n");
		return NULL;
	}

	Inst->CmdBuf = (XAie_TxnCmd *)calloc(TmpInst->NumCmds,
			sizeof(*Inst->CmdBuf));
	if(Inst->CmdBuf == NULL) {
		XAIE_ERROR("Failed to allocate memory for command buffer\n");
		free(Inst);
		return NULL;
	}

	Inst->CmdBuf = (XAie_TxnCmd *)memcpy((void *)Inst->CmdBuf,
			(void *)TmpInst->CmdBuf,
			TmpInst->NumCmds * sizeof(*Inst->CmdBuf));

	for(u32 i = 0U; i < TmpInst->NumCmds; i++) {
		XAie_TxnCmd *TmpCmd = &TmpInst->CmdBuf[i];
		XAie_TxnCmd *Cmd = &Inst->CmdBuf[i];

		if(TmpCmd->Opcode == XAIE_IO_BLOCKWRITE) {
			Cmd->DataPtr = (u64)(uintptr_t)malloc(
					sizeof(u32) * TmpCmd->Size);
			if((void *)(uintptr_t)Cmd->DataPtr == NULL) {
				XAIE_ERROR("Failed to allocate memory to copy "
						"command %d\n", i);
				/* Free previously allocated DataPtr memory to prevent leak */
				for(u32 j = 0U; j < i; j++) {
					if(Inst->CmdBuf[j].Opcode == XAIE_IO_BLOCKWRITE) {
						free((void *)(uintptr_t)Inst->CmdBuf[j].DataPtr);
					}
				}
				free(Inst->CmdBuf);
				free(Inst);
				return NULL;
			}

			Cmd->DataPtr = (u64)(uintptr_t)memcpy(
					(void *)(uintptr_t)Cmd->DataPtr,
					(void *)TmpCmd->DataPtr,
					sizeof(u32) * TmpCmd->Size);
		}
	}

	Inst->Tid = TmpInst->Tid;
	Inst->Flags = TmpInst->Flags;
	Inst->Flags |= XAIE_TXN_INSTANCE_EXPORTED;
	Inst->NumCmds = TmpInst->NumCmds;
	Inst->MaxCmds = TmpInst->MaxCmds;
	Inst->Node.Next = NULL;

	return Inst;
}

void BuffHexDump(char* buff,u32 size) {
	XAIE_DBG("Buff Info %p %d\n",buff,size);
	for (u32 i = 0; i < size; ++i) {
		printf("0x%x ",(u8)buff[i]&0xffU);
	}
	printf("\n");
}

static int TxnCmdDump(XAie_TxnCmd* cmd) {
	XAIE_DBG("TxnCmdDump Called for %d and size %d\n",cmd->Opcode,cmd->Size);
	BuffHexDump((char *)(cmd->DataPtr),cmd->Size);
	return 0;
}

/*****************************************************************************/
/**
* This API rellaocates the command buffer associated with the given transaction
* instance. The command buffer is extended to accomodate an additional
* TxnInst->InitCmds (XAIE_DEFAULT_NUM_CMDS number of commands by default else
* based on the value configured by application layer).
*
* @param        TxnInst: Pointer to the transaction instance
*
* @return       XAIE_OK on success and XAIE_ERR on failure
*
* @note         Internal only.
*
******************************************************************************/
static AieRC _XAie_ReallocCmdBuf(XAie_TxnInst *TxnInst)
{
	u64 NewMaxCmds = (u64)TxnInst->MaxCmds +TxnInst->InitCmds;
	if(NewMaxCmds > UINT32_MAX) {
		XAIE_ERROR("Failed reallocate memory for transaction buffer\n");
		return XAIE_ERR;
	}

	XAie_TxnCmd *TmpBuf = (XAie_TxnCmd *)realloc((void *)TxnInst->CmdBuf,
			sizeof(XAie_TxnCmd) * (u32)NewMaxCmds);
	if(TmpBuf == NULL) {
		XAIE_ERROR("Failed reallocate memory for transaction buffer "
				"with id: %llu\n", TxnInst->Tid);
		free(TxnInst->CmdBuf);  /* Free original memory to prevent leak */
		TxnInst->CmdBuf = NULL;
		return XAIE_ERR;
	}

	TxnInst->CmdBuf = TmpBuf;
	TxnInst->MaxCmds += TxnInst->InitCmds;
	XAIE_DBG(" Reallocated TXN CMD ARRAY to %llu with id: %llu\n", NewMaxCmds, TxnInst->Tid);

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API decodes the command type and executes the IO operation.
*
* @param        DevInst: Device instance pointer
* @param        Cmd: Pointer to the transaction command structure
* @param	Flags: Transaction instance flags
*
* @return       XAIE_OK on success and XAIE_ERR on failure.
*
* @note         Internal only.
*
******************************************************************************/
static AieRC _XAie_ExecuteCmd(XAie_DevInst *DevInst, XAie_TxnCmd *Cmd,
	u32 Flags)
{
AieRC RC;
const XAie_Backend *Backend = DevInst->Backend;

if(Cmd->Opcode >= XAIE_IO_CUSTOM_OP_BEGIN) {
	// TBD hooking point for  custom op handler
	XAIE_WARN("Custom OP Transaction %d handler hook point\n",Cmd->Opcode);
	return XAIE_OK;
}

switch(Cmd->Opcode)
{
	case XAIE_IO_WRITE:
		if(Cmd->Mask == 0U) {
			RC = Backend->Ops.Write32((void*)DevInst->IOInst,
					Cmd->RegOff, Cmd->Value);
		} else {

			RC = Backend->Ops.MaskWrite32((void*)DevInst->IOInst,
						Cmd->RegOff, Cmd->Mask,
						Cmd->Value);
		}
		if(RC != XAIE_OK) {
			XAIE_ERROR("Wr failed. Addr: 0x%llx, Mask: 0x%x,"
					"Value: 0x%x\n", Cmd->RegOff,
					Cmd->Mask, Cmd->Value);
			return RC;
		}
		break;
	case XAIE_IO_BLOCKWRITE:
		RC = Backend->Ops.BlockWrite32((void *)DevInst->IOInst,
				Cmd->RegOff,
				(u32 *)(uintptr_t)Cmd->DataPtr,
				Cmd->Size);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Block Wr failed. Addr: 0x%llx\n",
					Cmd->RegOff);
			return RC;
		}

		if((Flags & XAIE_TXN_INST_EXPORTED_MASK) == 0U) {
			free((void *)(uintptr_t)Cmd->DataPtr);
		}
		break;
	case XAIE_IO_BLOCKSET:
		RC = Backend->Ops.BlockSet32((void *)DevInst->IOInst,
				Cmd->RegOff, Cmd->Value,
				Cmd->Size);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Block Wr failed. Addr: 0x%llx\n",
					Cmd->RegOff);
			return RC;
		}
		break;
	case XAIE_IO_MASKPOLL:
	case XAIE_IO_MASKPOLL_BUSY:
		/* Force timeout to default value */
		RC = Backend->Ops.MaskPoll((void*)DevInst->IOInst,
						Cmd->RegOff, Cmd->Mask,
						Cmd->Value, 0U);
		if(RC != XAIE_OK) {
			XAIE_ERROR("MP failed. Addr: 0x%llx, Mask: 0x%x, Value: 0x%x\n",
					Cmd->RegOff, Cmd->Mask,
					Cmd->Value);
			return RC;
		}
		break;

	default:
		XAIE_ERROR("Invalid transaction opcode\n");
		return XAIE_ERR;
}

return XAIE_OK;
}

/*****************************************************************************/
/**
* This API executes all the commands in the command buffer and resets the number
* of commands.
*
* @param        DevInst: Device instance pointer
* @param        TxnInst: Pointer to the transaction instance
*
* @return       XAIE_OK on success and XAIE_ERR on failure
*
* @note         Internal only. This API does not allocate, reallocate or free
*		any buffer.
*
******************************************************************************/
static AieRC _XAie_Txn_FlushCmdBuf(XAie_DevInst *DevInst, XAie_TxnInst *TxnInst)
{
	AieRC RC;
	const XAie_Backend *Backend = DevInst->Backend;

	XAIE_DBG("Flushing %d commands from transaction buffer\n",
			TxnInst->NumCmds);

	if(Backend->Ops.SubmitTxn != NULL) {
		return Backend->Ops.SubmitTxn(DevInst->IOInst, TxnInst);
	}

	for(u32 i = 0U; i < TxnInst->NumCmds; i++) {
		RC = _XAie_ExecuteCmd(DevInst, &TxnInst->CmdBuf[i],
				TxnInst->Flags);
		if (RC != XAIE_OK) {
			 return RC;
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API frees transaction buffer if the instance is available.
*
* @param        DevInst: Device instance pointer
*
* @return       XAIE_OK on success and XAIE_ERR on failure
*
* @note         Internal only. This API does not allocate, reallocate
*		any buffer.
*
******************************************************************************/
AieRC _XAie_ClearTransaction(XAie_DevInst* DevInst)
{
	AieRC RC;
	XAie_TxnInst *Inst;
	const XAie_Backend *Backend = DevInst->Backend;

	Inst = _XAie_GetTxnInst(DevInst, Backend->Ops.GetTid());
	if(Inst == NULL) {
		XAIE_ERROR("Failed to get the correct transaction instance "
				"from internal list\n");
		return XAIE_ERR;
	}

	for(u32 i = 0U; i < Inst->NumCmds; i++) {
		XAie_TxnCmd *Cmd = &Inst->CmdBuf[i];

		if((Cmd->Opcode == XAIE_IO_BLOCKWRITE ||
			(Cmd->Opcode >= XAIE_IO_CUSTOM_OP_BEGIN && Cmd->Opcode < XAIE_IO_CUSTOM_OP_NEXT)) &&
			((void *)(uintptr_t)Cmd->DataPtr != NULL)) {
			free((void *)Cmd->DataPtr);
		}
	}

	RC = _XAie_RemoveTxnInstFromList(DevInst, Backend->Ops.GetTid());
	if(RC != XAIE_OK) {
		return RC;
	}

	free(Inst->CmdBuf);
	free(Inst);

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This API executes all the commands in the command buffer and frees the
* transaction instance unless it is exported to the user.
*
* @param        DevInst: Device instance pointer
* @param        TxnInst: Pointer to the transaction instance
*
* @return       XAIE_OK on success and XAIE_ERR on failure
*
* @note         Internal only.
*
******************************************************************************/
AieRC _XAie_Txn_Submit(XAie_DevInst *DevInst, XAie_TxnInst *TxnInst)
{
	AieRC RC;
	u64 Tid = 0;
	XAie_TxnInst *Inst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(TxnInst == NULL) {
		Tid = Backend->Ops.GetTid();
		Inst = _XAie_GetTxnInst(DevInst, Tid);
		if(Inst == NULL) {
			XAIE_ERROR("Failed to get the correct transaction "
					"instance\n");
			return XAIE_ERR;
		}
	} else {
		if(TxnInst->Flags & XAIE_TXN_INST_EXPORTED_MASK) {
			Inst = TxnInst;
		} else {
			XAIE_ERROR("Transaction instance was not exported.\n");
			return XAIE_ERR;
		}
	}

	RC =  _XAie_Txn_FlushCmdBuf(DevInst, Inst);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Flushing the buffer failed\n");
		return RC;
	}

	/* Do not free resources if transaction is exported */
	if(Inst->Flags & XAIE_TXN_INST_EXPORTED_MASK) {
		return XAIE_OK;
	}

	RC = _XAie_RemoveTxnInstFromList(DevInst, Tid);
	if(RC != XAIE_OK) {
		return RC;
	}

	free(Inst->CmdBuf);
	free(Inst);
	return XAIE_OK;
}


static inline void _XAie_AppendWrite32(XAie_DevInst *DevInst,
		XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_Write32Hdr *Hdr = (XAie_Write32Hdr*)(uintptr_t)TxnPtr;
	Hdr->RegOff = Cmd->RegOff;
	Hdr->Value = Cmd->Value;
	Hdr->Size = (u32)sizeof(*Hdr);
	Hdr->OpHdr.Col = _XAie_GetColfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Row = _XAie_GetRowfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Op = (u8)XAIE_IO_WRITE;
}

static inline void _XAie_AppendMaskWrite32(XAie_DevInst *DevInst,
		XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_MaskWrite32Hdr *Hdr = (XAie_MaskWrite32Hdr*)(uintptr_t)TxnPtr;

	Hdr->RegOff = Cmd->RegOff;
	Hdr->Mask = Cmd->Mask;
	Hdr->Value = Cmd->Value;
	Hdr->Size = (u32)sizeof(*Hdr);
	Hdr->OpHdr.Col = _XAie_GetColfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Row = _XAie_GetRowfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Op = (u8)XAIE_IO_MASKWRITE;
}

static inline void _XAie_AppendMaskPoll32(XAie_DevInst *DevInst,
		XAie_TxnCmd *Cmd, uint8_t *TxnPtr)
{
	XAie_MaskPoll32Hdr *Hdr = (XAie_MaskPoll32Hdr*)(uintptr_t)TxnPtr;

	Hdr->RegOff = Cmd->RegOff;
	Hdr->Mask = Cmd->Mask;
	Hdr->Value = Cmd->Value;
	Hdr->Size = (u32)sizeof(*Hdr);
	Hdr->OpHdr.Col = _XAie_GetColfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Row = _XAie_GetRowfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Op = (u8)XAIE_IO_MASKPOLL;
}

static inline void _XAie_AppendMaskPollBusy32(XAie_DevInst *DevInst,
		XAie_TxnCmd *Cmd, uint8_t *TxnPtr)
{
	XAie_MaskPoll32Hdr *Hdr = (XAie_MaskPoll32Hdr*)(uintptr_t)TxnPtr;

	Hdr->RegOff = Cmd->RegOff;
	Hdr->Mask = Cmd->Mask;
	Hdr->Value = Cmd->Value;
	Hdr->Size = (u32)sizeof(*Hdr);
	Hdr->OpHdr.Col = _XAie_GetColfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Row = _XAie_GetRowfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Op = (u8)XAIE_IO_MASKPOLL_BUSY;
}

static inline void _XAie_AppendBlockWrite32(XAie_DevInst *DevInst,
		XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	u8 *Payload = TxnPtr + sizeof(XAie_BlockWrite32Hdr);
	XAie_BlockWrite32Hdr *Hdr = (XAie_BlockWrite32Hdr*)(uintptr_t)TxnPtr;

	Hdr->RegOff = (u32)Cmd->RegOff;
	Hdr->Size = (u32)sizeof(*Hdr) + Cmd->Size * (u32)sizeof(u32);
	Hdr->OpHdr.Col = _XAie_GetColfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Row = _XAie_GetRowfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Op = (u8)XAIE_IO_BLOCKWRITE;

	memcpy((void *)Payload, (void *)Cmd->DataPtr,
			Cmd->Size * sizeof(u32));
}

static inline void _XAie_AppendBlockSet32(XAie_DevInst *DevInst,
		XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	u8 *Payload = TxnPtr + sizeof(XAie_BlockWrite32Hdr);
	XAie_BlockWrite32Hdr *Hdr = (XAie_BlockWrite32Hdr*)(uintptr_t)TxnPtr;

	Hdr->RegOff = (u32)Cmd->RegOff;
	Hdr->Size = (u32)sizeof(*Hdr) + Cmd->Size * (u32)sizeof(u32);
	Hdr->OpHdr.Col = _XAie_GetColfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Row = _XAie_GetRowfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Op = (u8)XAIE_IO_BLOCKWRITE;

	for (u32 i = 0U; i < Cmd->Size; i++) {
		*((u32 *)(uintptr_t)Payload) = Cmd->Value;
		Payload += 4;
	}
}

static inline void _XAie_AppendCustomOp(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	u8 *Payload = TxnPtr + sizeof(XAie_CustomOpHdr);
	XAie_CustomOpHdr *Hdr = (XAie_CustomOpHdr*)(uintptr_t)TxnPtr;

	Hdr->Size = (u32)sizeof(*Hdr) + Cmd->Size;
	Hdr->OpHdr.Op = (u8)Cmd->Opcode;

	for (u32 i = 0U; i < Cmd->Size; ++i, ++Payload) {
		*(Payload) = *((u8*)(uintptr_t)(Cmd->DataPtr + i));
	}
}

static inline void _XAie_AppendNoOp(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_NoOpHdr *Hdr = ( XAie_NoOpHdr*)(uintptr_t)TxnPtr;

	Hdr->Op = (u8)Cmd->Opcode;
}

static inline void _XAie_AppendPreempt(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_PreemptHdr *Hdr = ( XAie_PreemptHdr*)(uintptr_t)TxnPtr;

	Hdr->Op = (u8)Cmd->Opcode;
	Hdr->Preempt_level = (u8)Cmd->Preempt_level;
}

static inline void _XAie_AppendLoadPdi(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_LoadPdiHdr *Hdr = ( XAie_LoadPdiHdr*)((void*)TxnPtr);

	Hdr->Op = (u8)Cmd->Opcode;
	Hdr->PdiId = Cmd->PdiId;
	Hdr->PdiSize = 0;
	Hdr->PdiAddress = 0;
}

static inline u8* _XAie_AppendPmLoad(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_PmLoadHdr *Hdr = ( XAie_PmLoadHdr*)((void*)TxnPtr);
	Hdr->Op = (u8)Cmd->Opcode;
	Hdr->PmLoadId = Cmd->PmId;
	for(u8 i=0;i<3;i++)
	{
		Hdr->LoadSequenceCount[i] = 0;
	}
	return (&(Hdr->LoadSequenceCount[0]));
}

static inline void _XAie_AppendCreateScratchpad(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_CreateScratchpadHdr *Hdr = ( XAie_CreateScratchpadHdr*)(uintptr_t)TxnPtr;

	Hdr->Op = (u8)Cmd->Opcode;
	Hdr->UsageType = (u8)Cmd->UsageType;
	Hdr->Size = (u32)Cmd->Size;
	Hdr->ScratchOffset = (u64)Cmd->ScratchOffset;
}

static inline void _XAie_AppendUpdateState(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_UpdateStateHdr *Hdr = (XAie_UpdateStateHdr *)((void*)TxnPtr);

	// Check pass Func value is not more than 8 bit value.
	if (Cmd->Func > UCHAR_MAX) {
		XAIE_ERROR("Func cannot be represented in 8bit system\n");
		return ;
	}

	Hdr->Op = (u8)Cmd->Opcode;
	Hdr->StateTableIdx = Cmd->StateTableIdx;
	Hdr->Func = (u8)Cmd->Func;
	Hdr->FuncArg = Cmd->FuncArg;
}

static inline void _XAie_AppendUpdateReg(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_UpdateRegHdr *Hdr = (XAie_UpdateRegHdr *)((void*)TxnPtr);
	u32 RegOff = Cmd->RegOff & 0xFFFFFFFF;

	// Check pass Func value is not more than 8 bit value.
	if (Cmd->Func > UCHAR_MAX) {
		XAIE_ERROR("Func cannot be represented in 8bit system\n");
		return ;
	}

	Hdr->Op = (u8)Cmd->Opcode;
	Hdr->RegOff = RegOff;
	Hdr->StateTableIdx = Cmd->StateTableIdx;
	Hdr->Func = (u8)Cmd->Func;
	Hdr->FuncArg = Cmd->FuncArg;
}

static inline void _XAie_AppendUpdateScratch(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_UpdateScratchHdr *Hdr = ( XAie_UpdateScratchHdr*)(uintptr_t)TxnPtr;

	Hdr->Op = (u8)Cmd->Opcode;
}

static inline void _XAie_CreateTxnHeader_opt(XAie_DevInst *DevInst,
		XAie_TxnHeader *Header)
{
	Header->Major = XAIE_TXN_VER_10_MAJOR;
	Header->Minor = XAIE_TXN_VER_10_MINOR;
	Header->DevGen = DevInst->DevProp.DevGen;
	Header->NumRows = DevInst->NumRows;
	Header->NumCols = DevInst->NumCols;
	Header->NumMemTileRows = DevInst->MemTileNumRows;
	XAIE_DBG("Header version %d.%d\n", Header->Major, Header->Minor);
	XAIE_DBG("Device Generation: %d\n", Header->DevGen);
	XAIE_DBG("Cols, Rows, MemTile rows : (%d, %d, %d)\n", Header->NumCols,
			Header->NumRows, Header->NumMemTileRows);
}

static inline void _XAie_AppendWrite32_opt(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_Write32Hdr_opt *Hdr = (XAie_Write32Hdr_opt*)(uintptr_t)TxnPtr;
	Hdr->RegOff = Cmd->RegOff;
	Hdr->Value = Cmd->Value;
	Hdr->OpHdr.Op = (u8)XAIE_IO_WRITE;
}

static inline void _XAie_AppendMaskWrite32_opt(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_MaskWrite32Hdr_opt *Hdr = (XAie_MaskWrite32Hdr_opt*)(uintptr_t)TxnPtr;

	Hdr->RegOff = Cmd->RegOff;
	Hdr->Mask = Cmd->Mask;
	Hdr->Value = Cmd->Value;
	Hdr->OpHdr.Op = (u8)XAIE_IO_MASKWRITE;
}

static inline void _XAie_AppendMaskPoll32_opt(XAie_TxnCmd *Cmd, uint8_t *TxnPtr)
{
	XAie_MaskPoll32Hdr_opt *Hdr = (XAie_MaskPoll32Hdr_opt*)(uintptr_t)TxnPtr;

	Hdr->RegOff = Cmd->RegOff;
	Hdr->Mask = Cmd->Mask;
	Hdr->Value = Cmd->Value;
	Hdr->OpHdr.Op = (u8)XAIE_IO_MASKPOLL;
}

static inline void _XAie_AppendMaskPollBusy32_opt(XAie_TxnCmd *Cmd, uint8_t *TxnPtr)
{
	XAie_MaskPoll32Hdr_opt *Hdr = (XAie_MaskPoll32Hdr_opt*)(uintptr_t)TxnPtr;

	Hdr->RegOff = Cmd->RegOff;
	Hdr->Mask = Cmd->Mask;
	Hdr->Value = Cmd->Value;
	Hdr->OpHdr.Op = (u8)XAIE_IO_MASKPOLL_BUSY;
}
static inline void _XAie_AppendBlockWrite32_opt(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	u32 *Payload = (void*)(TxnPtr + sizeof(XAie_BlockWrite32Hdr_opt));
	XAie_BlockWrite32Hdr_opt *Hdr = (XAie_BlockWrite32Hdr_opt*)(uintptr_t)TxnPtr;

	Hdr->RegOff = (u32)Cmd->RegOff;
	Hdr->Size = (u32)sizeof(*Hdr) + Cmd->Size * (u32)sizeof(u32);
	Hdr->OpHdr.Op = (u8)XAIE_IO_BLOCKWRITE;

	memcpy((void *)Payload, (void *)Cmd->DataPtr,
			Cmd->Size * sizeof(u32));
}

static inline void _XAie_AppendBlockSet32_opt(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	u8 *Payload = TxnPtr + sizeof(XAie_BlockWrite32Hdr_opt);
	XAie_BlockWrite32Hdr_opt *Hdr = (XAie_BlockWrite32Hdr_opt*)(uintptr_t)TxnPtr;

	Hdr->RegOff = (u32)Cmd->RegOff;
	Hdr->Size = (u32)sizeof(*Hdr) + Cmd->Size * (u32)sizeof(u32);
	Hdr->OpHdr.Op = (u8)XAIE_IO_BLOCKWRITE;

	for (u32 i = 0U; i < Cmd->Size; i++) {
		*((u32 *)(uintptr_t)Payload) = Cmd->Value;
		Payload += 4;
	}
}

static inline void _XAie_AppendCustomOp_opt(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	u8 *Payload = TxnPtr + sizeof(XAie_CustomOpHdr_opt);
	XAie_CustomOpHdr_opt *Hdr = (XAie_CustomOpHdr_opt*)(uintptr_t)TxnPtr;

	Hdr->Size = (u32)sizeof(*Hdr) + Cmd->Size;
	Hdr->OpHdr.Op = (u8)Cmd->Opcode;

	for (u32 i = 0U; i < Cmd->Size; ++i, ++Payload) {
		*(Payload) = *((u8*)(uintptr_t)(Cmd->DataPtr + i));
	}
}

static inline void _XAie_AppendDDRPatch_opt(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	u8 *Payload = TxnPtr + sizeof(XAie_CustomOpHdr_opt);

	/**
	 * Use stack variable instead of pointer access to avoid unaligned access
	 * when TxnPtr is unaligned. To avoid UBSan alignment errors.
	 */
	XAie_CustomOpHdr_opt Hdr;
	patch_op_t PatchOp;
	patch_op_opt_t PatchOpOpt;

	// Modify the cmd size to align with version 1.0
	Cmd->Size = (u32)sizeof(patch_op_opt_t);

	// Write the custom header into Txn Ptr with correct size.
	Hdr.Size = (u32)sizeof(Hdr) + Cmd->Size;
	Hdr.OpHdr.Op = (u8)Cmd->Opcode;
	Hdr.OpHdr.padding[0] = 0;
	Hdr.OpHdr.padding[1] = 0;
	Hdr.OpHdr.padding[2] = 0;

	// Use memcpy to avoid unaligned access
	memcpy(TxnPtr, &Hdr, sizeof(Hdr));

#if UINTPTR_MAX == U64_MAX  // 64-bit system
    if (Cmd->DataPtr > UINTPTR_MAX) {
    	XAIE_ERROR("DataPtr cannot be represented in 64bit system\n");
    	return ;
    }
#endif

	// Map the patch_op_t fields to patch_op_opt_t
	// Use memcpy to safely read from DataPtr, which may point to misaligned data.
	// memcpy performs byte-wise copy and does not require alignment.
	// Coverity note: The cast to void* is intentional. The alignment of the
	// source pointer is unknown, but memcpy handles misalignment correctly.
	const void *data_src = (const void *)(uintptr_t)Cmd->DataPtr;
	memcpy(&PatchOp, data_src, sizeof(PatchOp));

	// Convert u64 fields to smaller types
	PatchOpOpt.regaddr = (u32)(PatchOp.regaddr & UINT32_MAX);
	PatchOpOpt.argidx = (u8)(PatchOp.argidx & UINT8_MAX);
	PatchOpOpt.padding[0] = 0;
	PatchOpOpt.padding[1] = 0;
	PatchOpOpt.padding[2] = 0;
	PatchOpOpt.argplus = PatchOp.argplus;

	// Write patch_op_opt_t into txn ptr using memcpy to avoid unaligned access
	memcpy(Payload, &PatchOpOpt, sizeof(PatchOpOpt));
}

static u8* _XAie_ReallocTxnBuf_MemInit(u8 *TxnPtr, u32 NewSize, u32 Buffsize)
{
	u8 *Tmp;
	Tmp =  (u8*)realloc((void*)TxnPtr, NewSize);
	if(Tmp == NULL) {
		XAIE_ERROR("Reallocation failed for txn buffer\n");
		free(TxnPtr);  /* Free original memory to prevent leak */
		return NULL;
	}
	memset(Tmp + Buffsize,0,(NewSize  - Buffsize));
	return Tmp;
}

static u8* _XAie_ReallocTxnBuf(u8 *TxnPtr, u32 NewSize)
{
	u8 *Tmp;
	Tmp =  (u8*)realloc((void*)TxnPtr, NewSize);
	if(Tmp == NULL) {
		XAIE_ERROR("Reallocation failed for txn buffer\n");
		free(TxnPtr);  /* Free original memory to prevent leak */
		return NULL;
	}

	return Tmp;
}

static inline void _XAie_AppendBWToBlockwriteBuff(XAie_DevInst *DevInst, XAie_TxnCmd *Cmd, u8 FirstBlockwriteProcessed, u32* BlockwriteBuffer)
{
	XAie_BlockWrite32Hdr *Hdr = (XAie_BlockWrite32Hdr*)(uintptr_t)BlockwriteBuffer;
	u32* Payload;
	u32 memcpy_size = Cmd->Size * (u32)sizeof(u32);
	#if UINTPTR_MAX == U64_MAX  // 64-bit system
    	if (Cmd->DataPtr > UINTPTR_MAX) {
    		XAIE_ERROR("DataPtr cannot be represented in 64 bit system\n");
    		return;
    }
    #endif
	if(FirstBlockwriteProcessed == 0)
	{
		u32 payload_offset = sizeof(XAie_BlockWrite32Hdr) / 4;
		Payload = (void *)(BlockwriteBuffer + payload_offset);

		u32 RegOff = Cmd->RegOff & UINT_MAX;
		Hdr->RegOff = RegOff;
		Hdr->Size = (u32)sizeof(XAie_BlockWrite32Hdr) + Cmd->Size * (u32)sizeof(u32);
		Hdr->OpHdr.Col = _XAie_GetColfromRegOff(DevInst,Cmd->RegOff);
		Hdr->OpHdr.Row = _XAie_GetRowfromRegOff(DevInst,Cmd->RegOff);
		Hdr->OpHdr.Op = (u8)XAIE_IO_BLOCKWRITE;

		memcpy((void*)Payload, (void const*)(uintptr_t)Cmd->DataPtr, memcpy_size);
	}
	else
	{
		Payload = (void *)(BlockwriteBuffer + (Hdr->Size / 4));
		memcpy((void*)Payload, (void const*)(uintptr_t)Cmd->DataPtr, memcpy_size);
		Hdr->Size +=Cmd->Size * (u32)sizeof(u32);
	}
}

static inline u32 _XAie_AppendBWToTxnBuff(u32* BlockwriteBuffer,u8* TxnPtr, u32 PatchCmdCount)
{
	XAie_BlockWrite32Hdr *Hdr = (XAie_BlockWrite32Hdr*)(uintptr_t)BlockwriteBuffer;
	u32 Size = 0,PatchCmdSize=0;
	u8* TempPtr = NULL;

	if(PatchCmdCount != 0){

		PatchCmdSize = ((PatchCmdCount) * ( sizeof(patch_op_t) + sizeof(XAie_CustomOpHdr) )) & UINT_MAX;
		XAIE_DBG("_XAie_AppendBWToTxnBuff:PatchCmdSize = %d\n", PatchCmdSize);

		TempPtr = calloc(1,PatchCmdSize);
		if(TempPtr == NULL) {
			XAIE_ERROR("Calloc failed\n");
			return 0;
		}
	
		TxnPtr -= PatchCmdSize;
		memcpy(TempPtr, TxnPtr, PatchCmdSize);
		memset(TxnPtr, 0, PatchCmdSize);
	}
	else
	{
		XAIE_DBG("PatchCmdCount is 0\n");
	}

	memcpy((void*)TxnPtr, (const void *)(uintptr_t)BlockwriteBuffer, Hdr->Size);
	Size = Hdr->Size;
	memset(BlockwriteBuffer, 0, Size);

	if(PatchCmdCount != 0)
	{
		TxnPtr += Size;
		memcpy(TxnPtr, TempPtr, PatchCmdSize);
		free(TempPtr);
	}
	else
	{
		XAIE_DBG("PatchCmdCount is 0\n");
	}

    return Size;
}

/*****************************************************************************/
/**
* This API checks if the current TXN command to be added to TXN buffer will
* cross the 16KB boundary and if does then calculates remaining bytes in TXN
* buffer before the next 16KB boundary.
*
* @param        CmdSize: Size of the command to be checked if it fits or spills
* @param        CurrentBuffSize: Current filled TXN buffer size
* @param        RemainingBytes: Pointer to return remaining bytes in TXN
*                               buffer before next 16KB boundary
*
* @return       1 if crosses 16KB boundary, 
*               0 if command fits within current page
*               (including cases where command exactly reaches the boundary)
*
* @note         Internal only.
*
******************************************************************************/
static inline u8 _XAie_CheckTxnCmdSpillsOver16KBBoundary(u32 CmdSize,
														 u32 CurrentBuffSize,
											  			 u32 *RemainingBytes)
{
	u32 CurBoundaryId = CurrentBuffSize / XAIE_TXN_16KB_BOUNDARY;
	u32 NewBoundaryId = (CurrentBuffSize + CmdSize) / XAIE_TXN_16KB_BOUNDARY;
	u32 NextBoundaryOffset = (CurBoundaryId + 1U) * XAIE_TXN_16KB_BOUNDARY;

	// Calculate remaining bytes before next 16KB boundary
	if (RemainingBytes != NULL) {
		*RemainingBytes = NextBoundaryOffset - CurrentBuffSize;
	}

	// Return 0 if CmdSize can fit into current page (exact boundary fit)
	if (NextBoundaryOffset == (CurrentBuffSize + CmdSize)) {
		XAIE_DBG("The Command Fits exactly\n");
		return 0U;
	}

	return (NewBoundaryId > CurBoundaryId) ? 1U : 0U;
}

/*****************************************************************************/
/**
* This function adds NoOp commands to fill remaining bytes to 16KB boundary.
*
* @param	TxnPtr: Pointer to transaction buffer location.
* @param	BuffSize: Pointer to current buffer size (updated by function).
* @param	AllocatedBuffSize: Pointer to allocated buffer size (updated by
*                             function).
* @param	RemainingBytes: Number of bytes remaining to 16KB boundary.
* @param	BlockwriteBuffer: Pointer to blockwrite buffer (for error cleanup).
* @param	NumOps: Pointer to number of operations (incremented by function).
*
* @return	Updated TxnPtr or NULL on error.
*
* @note	Internal only.
*
******************************************************************************/
static inline u8* _XAie_AddNoOpPadding16KB(u8 *TxnPtr,
					    u32 *BuffSize, u32 *AllocatedBuffSize,
					    u32 RemainingBytes, u32 *BlockwriteBuffer,
					    u32 *NumOps)
{
	u32 NoOpCmdsNeeded = RemainingBytes / sizeof(XAie_NoOpHdr);
	u32 UnAlignedAddr = RemainingBytes % sizeof(XAie_NoOpHdr);
	XAie_NoOpHdr NoOpCmd = {
		.Op = XAIE_IO_NOOP,
		.padding = {0,0,0}
	};

	/**
	 * Kotesh(TODO): Check if this is a valid case and if yes then does the 
	 *               current handling suffices
	 */
	if (UnAlignedAddr != 0) {
		XAIE_ERROR("**** TxnPtr is not aligned even after NoOp padding\n");
		if (BlockwriteBuffer != NULL) {
			free(BlockwriteBuffer);
		}

		u8 *TxnPtrStart = TxnPtr - *BuffSize;
		free(TxnPtrStart);
		return NULL;
	}

	// Add required no of TXN NOOP commands
	for (u32 NoOpIdx = 0U; NoOpIdx < NoOpCmdsNeeded; NoOpIdx++) {
		// Ensure buffer has enough space for NoOp
		while ((*BuffSize + sizeof(XAie_NoOpHdr)) > *AllocatedBuffSize) {
			TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - *BuffSize,
					*AllocatedBuffSize * 2U, *BuffSize);
			if (TxnPtr == NULL) {
				XAIE_ERROR("TxnPtr realloc failed during NoOp padding\n");
				if (BlockwriteBuffer != NULL) {
					free(BlockwriteBuffer);
				}
				return NULL;
			}
			*AllocatedBuffSize *= 2U;
			TxnPtr += *BuffSize;
		}

		XAIE_DBG("Added %d NoOps\n", NoOpCmdsNeeded);

		// Directly copy the NoOp header instead of unsafe casting
		XAie_NoOpHdr *Hdr = (XAie_NoOpHdr*)(uintptr_t)TxnPtr;
		*Hdr = NoOpCmd;
		TxnPtr += sizeof(XAie_NoOpHdr);
		*BuffSize += (u32)sizeof(XAie_NoOpHdr);

		// Increment the number of operations for each NoOp added
		if (NumOps != NULL) {
			(*NumOps)++;
		}
	}

	return TxnPtr;
}

/*****************************************************************************/
/**
* This function checks if a given transaction command to be added into TXN
* buffer will cross a 16KB boundary. If yes the will adds TXN NoOp commands
* as padding till 16KB boundary. SO that current command can be added without
* spilling over the 16KB boundary.
*
* @param        TxnPtr: Pointer to current position in transaction buffer
* @param        BuffSize: Pointer to current buffer size
* @param        AllocatedBuffSize: Pointer to allocated buffer size
* @param        CmdSize: Size of the command to check
* @param        BlockwriteBuffer: Block write buffer for cleanup on failure
* @param        NumOps: Pointer to number of operations counter
*
* @return       Updated TxnPtr on success, NULL on failure
*
* @note         Internal only.
*
******************************************************************************/
static inline u8* _XAie_HandleTxnCmd16KBBoundary(u8 *TxnPtr, u32 *BuffSize,
												 u32 *AllocatedBuffSize,
						 						 u32 CmdSize,
												 u32 *BlockwriteBuffer,
												 u32 *NumOps)
{
	u32 RemainingBytes = 0U;

	if (_XAie_CheckTxnCmdSpillsOver16KBBoundary(CmdSize, *BuffSize, &RemainingBytes)) {
		XAIE_DBG("TXN command of size %u will spill across 16KB boundary.\n",
			 CmdSize);
		XAIE_DBG("With cur buffer size %u, only %u bytes are availble\n",
			 *BuffSize, RemainingBytes);

		// Add NoOp commands to fill remaining bytes to 16KB boundary
		TxnPtr = _XAie_AddNoOpPadding16KB(TxnPtr, BuffSize,
						AllocatedBuffSize, RemainingBytes,
						BlockwriteBuffer, NumOps);
		if (TxnPtr == NULL) {
			return NULL;
		}
	}

	return TxnPtr;
}

/*****************************************************************************/
/**
*
* This api copies an existing transaction instance and returns a copy of the
* instance with all the commands for users to save the commands and use them
* at a later point.
*
* @param	DevInst - Device instance pointer.
* @param	NumConsumers - Number of consumers for the generated
*		transactions (Unused for now)
* @param	Flags - Flags (Unused for now)
*
* @return	Pointer to copy of transaction instance on success and NULL
*		on error.
*
* @note		Internal only.
*
******************************************************************************/
u8* _XAie_TxnExportSerialized(XAie_DevInst *DevInst, u8 NumConsumers,
		u32 Flags)
{
	const XAie_Backend *Backend = DevInst->Backend;
	XAie_TxnInst *TmpInst = NULL;
	u8 *TxnPtr = NULL;
	u32 BuffSize = 0U, NumOps = 0, PatchCmdCount = 0,BWBuffSize = 0;
	u32 LoadSeqCount = 0;
	u64 RegOffLastBlockWrite = 0;
	u8 FirstBlockwriteProcessed = 0;
	u32* BlockwriteBuffer = NULL;
	u8* LoadSeqCountPtr = NULL;
	u32 LoadSeqCountOffset = 0;

	u32 AllocatedBuffSize = XAIE_DEFAULT_TXN_BUFFER_SIZE;
	u32 BWBuffAllocatedSize = XAIE_DEFAULT_TXN_BUFFER_SIZE;
	(void)NumConsumers;
	(void)Flags;

	TmpInst = _XAie_GetTxnInst(DevInst, Backend->Ops.GetTid());
	if(TmpInst == NULL) {
		XAIE_ERROR("Failed to get the correct transaction instance "
				"from internal list\n");
		return NULL;
	}

	BlockwriteBuffer = calloc(1,BWBuffAllocatedSize);
	if(BlockwriteBuffer == NULL) {
		XAIE_ERROR("BlockWrite Buffer Calloc failed\n");
		return NULL;
	}

	TxnPtr = calloc(1,AllocatedBuffSize);
	if(TxnPtr == NULL) {
		XAIE_ERROR("TxnPtr Calloc failed\n");
		free(BlockwriteBuffer);
		return NULL;
	}

	_XAie_CreateTxnHeader(DevInst, (XAie_TxnHeader *)(uintptr_t)TxnPtr);
	BuffSize += (u32)sizeof(XAie_TxnHeader);
	TxnPtr += sizeof(XAie_TxnHeader);
	XAIE_DBG("number of cmd %d\n", TmpInst->NumCmds);

	for(u32 i = 0U; i < TmpInst->NumCmds; i++) {
		NumOps++;
		XAie_TxnCmd *Cmd = &TmpInst->CmdBuf[i];

		if( (DevInst->PmLoadingActive == 1) && (Cmd->Opcode != XAIE_IO_LOAD_PM_START) && (Cmd->Opcode != XAIE_IO_LOAD_PM_END_INTERNAL))
		{
			LoadSeqCount++;
		}

		if( (Cmd->Opcode != XAIE_IO_BLOCKWRITE) &&
		    (Cmd->Opcode != XAIE_IO_CUSTOM_OP_DDR_PATCH) &&
		    (FirstBlockwriteProcessed != 0) )
		{
			XAie_BlockWrite32Hdr *Hdr = (XAie_BlockWrite32Hdr*)BlockwriteBuffer;
			while((BuffSize + Hdr->Size) > AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
				(AllocatedBuffSize) * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			BuffSize += Hdr->Size;
			TxnPtr += _XAie_AppendBWToTxnBuff(BlockwriteBuffer,TxnPtr,PatchCmdCount);
			PatchCmdCount = 0;
			FirstBlockwriteProcessed = 0;
		}
		if ((Cmd->Opcode == XAIE_IO_WRITE) && (Cmd->Mask == 0U)) {
			while((BuffSize + sizeof(XAie_Write32Hdr)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL){
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendWrite32(DevInst, Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_Write32Hdr);
			BuffSize += (u32)sizeof(XAie_Write32Hdr);
			continue;
		}
		else if ((Cmd->Opcode == XAIE_IO_WRITE) && ((Cmd->Mask)!=0U)) {
			while((BuffSize + sizeof(XAie_MaskWrite32Hdr)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendMaskWrite32(DevInst, Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_MaskWrite32Hdr);
			BuffSize += (u32)sizeof(XAie_MaskWrite32Hdr);
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_MASKPOLL) {
			while((BuffSize + sizeof(XAie_MaskPoll32Hdr)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendMaskPoll32(DevInst, Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_MaskPoll32Hdr);
			BuffSize += (u32)sizeof(XAie_MaskPoll32Hdr);
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_MASKPOLL_BUSY) {
			while((BuffSize + sizeof(XAie_MaskPoll32Hdr)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendMaskPollBusy32(DevInst, Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_MaskPoll32Hdr);
			BuffSize += (u32)sizeof(XAie_MaskPoll32Hdr);
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_BLOCKWRITE) {
			/**
			 * In case of Block Write and Block Set, it is possible
			 * that the new allocated buffer size may not be sufficient.
			 * In that case we should keep reallocating till the new
			 * buffer size if big enough to hold existing + current opcode.
			 */
			BWBuffSize = ((XAie_BlockWrite32Hdr*)(uintptr_t)BlockwriteBuffer)->Size;
			if(FirstBlockwriteProcessed != 0)
			{
			 	if ( Cmd->RegOff != RegOffLastBlockWrite)
			 	{
					while((BuffSize + BWBuffSize) > AllocatedBuffSize) {
						TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						(AllocatedBuffSize) * 2U, BuffSize);
						if(TxnPtr == NULL) {
							XAIE_ERROR("TxnPtr realloc failed\n");
							free(BlockwriteBuffer);
							return NULL;
						}
						AllocatedBuffSize *= 2U;
						TxnPtr += BuffSize;
					}
					BuffSize += BWBuffSize;
					TxnPtr += _XAie_AppendBWToTxnBuff(BlockwriteBuffer,TxnPtr,PatchCmdCount);
					PatchCmdCount = 0;
					FirstBlockwriteProcessed = 0;
				}
				else
				{
					NumOps--;

					/**
					 * Load Sequence Count should be reduced only if there is an active
					 * PM Loading else do nothing.
					 **/
					if (DevInst->PmLoadingActive == 1) {
						LoadSeqCount--;
					}
				}
			} else {
				/**
				 *  When BW Buffer is empty which is represented by FirstBlockwriteProcessed == 0
				 *  BWBuffSize should be initialized to size of XAie_BlockWrite32Hdr struct in bytes.
				 *  Since the Cmd->Size field of Cmd from TmpInst->CmdBuf only considers payload size.
				 */
				BWBuffSize = sizeof(XAie_BlockWrite32Hdr);
			}

			while( (Cmd->Size * 4) + BWBuffSize > BWBuffAllocatedSize)
			{
				BlockwriteBuffer = (u32*) (uintptr_t) ( _XAie_ReallocTxnBuf_MemInit((u8 *)BlockwriteBuffer,
									(BWBuffAllocatedSize + BWBuffAllocatedSize), BWBuffSize) );
				if(BlockwriteBuffer == NULL) {
					XAIE_ERROR("BlockWrite Buffer Realloc Failed\n");
					// To free Txn Buffer successfully we need to use the start pointer not current pointer
					free(TxnPtr - BuffSize);
					return NULL;
				}
				BWBuffAllocatedSize *= 2U;
			}
			RegOffLastBlockWrite = (u64) ( Cmd->RegOff + (Cmd->Size*4) );
			_XAie_AppendBWToBlockwriteBuff(DevInst, Cmd, FirstBlockwriteProcessed, BlockwriteBuffer);
			FirstBlockwriteProcessed = 1;
		}
		else if (Cmd->Opcode == XAIE_IO_BLOCKSET) {
			/**
			 * In case of Block Write and Block Set, it is possible
			 * that the new allocated buffer size may not be sufficient
			 * In that case we should keep reallocating till the new
			 * buffer size if big enough to hold existing + current opcode.
			 *
			 * Blockset gets converted to blockwrite. so check for
			 * blockwrite size
			 */
			while((BuffSize + sizeof(XAie_BlockWrite32Hdr) +
						Cmd->Size * sizeof(u32)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendBlockSet32(DevInst, Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_BlockWrite32Hdr) +
				Cmd->Size * sizeof(u32);
			BuffSize += (u32)sizeof(XAie_BlockWrite32Hdr) +
				Cmd->Size * (u32)sizeof(u32);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_NOOP)
		{
			while( (BuffSize + sizeof(XAie_NoOpHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendNoOp(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_NoOpHdr);
			BuffSize += (u32)sizeof(XAie_NoOpHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_PREEMPT)
		{
			while( (BuffSize + sizeof(XAie_PreemptHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendPreempt(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_PreemptHdr);
			BuffSize += (u32)sizeof(XAie_PreemptHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_LOADPDI)
		{
			while( (BuffSize + sizeof(XAie_LoadPdiHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendLoadPdi(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_LoadPdiHdr);
			BuffSize += (u32)sizeof(XAie_LoadPdiHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_LOAD_PM_START)
		{
			while( (BuffSize + sizeof(XAie_PmLoadHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			LoadSeqCountPtr = _XAie_AppendPmLoad(Cmd, TxnPtr);
			LoadSeqCountOffset = LoadSeqCountPtr - (TxnPtr - BuffSize);
			TxnPtr += sizeof(XAie_PmLoadHdr);
			BuffSize += (u32)sizeof(XAie_PmLoadHdr);
			DevInst->PmLoadingActive = 1;
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_LOAD_PM_END_INTERNAL)
		{
			// Check if LoadSeqCountPtr needs to be updated due to realloc of TXN Ptr.
			if ((LoadSeqCountOffset != 0) && (LoadSeqCountPtr != ((TxnPtr - BuffSize) + LoadSeqCountOffset))) {
				LoadSeqCountPtr = (TxnPtr - BuffSize) + LoadSeqCountOffset;
			}

			if((LoadSeqCount != 0) && (LoadSeqCountPtr != NULL))
			{
				*LoadSeqCountPtr = LoadSeqCount & 0xFF;
				*(LoadSeqCountPtr + 1) = (LoadSeqCount & 0xFF00) >> 8;
				*(LoadSeqCountPtr + 2) = (LoadSeqCount & 0xFF0000) >> 16;
			}
			else if (LoadSeqCountPtr == NULL)
			{
				XAIE_ERROR("LoadSeqCountPtr is equal to NULL\n");
				free(BlockwriteBuffer);
				// To free Txn Buffer successfully we need to use the start pointer not current pointer
				free(TxnPtr - BuffSize);
				return NULL;
			}
			LoadSeqCount = 0;
			NumOps--;
			DevInst->PmLoadingActive = 0;
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_CREATE_SCRATCHPAD)
		{
			while( (BuffSize + sizeof(XAie_CreateScratchpadHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendCreateScratchpad(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_CreateScratchpadHdr);
			BuffSize += (u32)sizeof(XAie_CreateScratchpadHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_UPDATE_STATE_TABLE)
		{
			while((BuffSize + sizeof(XAie_UpdateStateHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendUpdateState(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_UpdateStateHdr);
			BuffSize += (u32)sizeof(XAie_UpdateStateHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_UPDATE_REG)
		{
			/* Check for 16KB boundary crossing and add padding if needed */
			TxnPtr = _XAie_HandleTxnCmd16KBBoundary(TxnPtr, &BuffSize,
								&AllocatedBuffSize, (u32)sizeof(XAie_UpdateRegHdr),
								BlockwriteBuffer, &NumOps);
			if (TxnPtr == NULL) {
				return NULL;
			}
			while((BuffSize + sizeof(XAie_UpdateRegHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendUpdateReg(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_UpdateRegHdr);
			BuffSize += (u32)sizeof(XAie_UpdateRegHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_UPDATE_SCRATCH)
		{
			while( (BuffSize + sizeof(XAie_UpdateScratchHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendUpdateScratch(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_UpdateScratchHdr);
			BuffSize += (u32)sizeof(XAie_UpdateScratchHdr);
			continue;
		}
		else if (Cmd->Opcode >= XAIE_IO_CUSTOM_OP_TCT) {

			if(Cmd->Opcode == XAIE_IO_CUSTOM_OP_DDR_PATCH)
			{
				++PatchCmdCount;
			}
			if (TX_DUMP_ENABLE) {
				TxnCmdDump(Cmd);
			}

			XAIE_DBG("Size of the CustomOp Hdr being exported: %u bytes\n", sizeof(XAie_CustomOpHdr));

			while((BuffSize + sizeof(XAie_CustomOpHdr) +
						Cmd->Size) > AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendCustomOp(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_CustomOpHdr) +
				Cmd->Size * sizeof(u8);
			BuffSize += (u32)sizeof(XAie_CustomOpHdr) +
				Cmd->Size * (u32)sizeof(u8);
			continue;
		}
	}
	if(FirstBlockwriteProcessed != 0)
	{	
		XAie_BlockWrite32Hdr *Hdr = (XAie_BlockWrite32Hdr*)(uintptr_t)BlockwriteBuffer;
		while((BuffSize + Hdr->Size) > AllocatedBuffSize) {
			TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						(AllocatedBuffSize) * 2U, BuffSize);
			if(TxnPtr == NULL) {
				XAIE_ERROR("TxnPtr realloc failed\n");
				free(BlockwriteBuffer);
				return NULL;
			}
			AllocatedBuffSize *= 2U;
			TxnPtr += BuffSize;
		}
		BuffSize += Hdr->Size;
		TxnPtr += _XAie_AppendBWToTxnBuff(BlockwriteBuffer,TxnPtr,PatchCmdCount);
	}

	// Free the BlockwriteBuffer
	free(BlockwriteBuffer);

	u32 four_byte_aligned_BuffSize = ((BuffSize % 4U) != 0U) ? ((BuffSize / 4U + 1U)*4) : BuffSize;
	XAIE_DBG("Size of the Txn Hdr being exported: %u bytes\n",
			sizeof(XAie_TxnHeader));
	XAIE_DBG("Size of the transaction buffer being exported: %u bytes\n",
			four_byte_aligned_BuffSize);
	XAIE_DBG("Num of Operations in the transaction buffer: %u\n",
			TmpInst->NumCmds);

	/* Adjust pointer and reallocate to the right size */
	TxnPtr = _XAie_ReallocTxnBuf(TxnPtr - BuffSize, four_byte_aligned_BuffSize);
	if(TxnPtr == NULL) {
		XAIE_ERROR("TxnPtr realloc failed\n");
		return NULL;
	}
	((XAie_TxnHeader *)(uintptr_t)TxnPtr)->NumOps =  NumOps;
	((XAie_TxnHeader *)(uintptr_t)TxnPtr)->TxnSize =  four_byte_aligned_BuffSize;

	return (u8 *)TxnPtr;
}

static inline void _XAie_AppendBWToBlockwriteBuff_opt(XAie_TxnCmd *Cmd, u8 FirstBlockwriteProcessed, u32* BlockwriteBuffer)
{
	XAie_BlockWrite32Hdr_opt *Hdr = (XAie_BlockWrite32Hdr_opt*)(uintptr_t)BlockwriteBuffer;
	u32* Payload;
	u32 memcpy_size = Cmd->Size * (u32)sizeof(u32);
	#if UINTPTR_MAX == U64_MAX  // 64-bit system
	if (Cmd->DataPtr > UINTPTR_MAX) {
		XAIE_ERROR("DataPtr cannot be represented in 64bit system\n");
		return;
	}
	#endif
	if(FirstBlockwriteProcessed == 0)
	{
		u32 payload_offset = sizeof(XAie_BlockWrite32Hdr_opt) / 4;
		Payload = (void *)(BlockwriteBuffer + payload_offset);

		u32 RegOff = Cmd->RegOff & UINT_MAX;
		Hdr->RegOff = RegOff;
		Hdr->Size = (u32)sizeof(XAie_BlockWrite32Hdr_opt) + Cmd->Size * (u32)sizeof(u32);
		Hdr->OpHdr.Op = (u8)XAIE_IO_BLOCKWRITE;

		memcpy((void*)Payload, (void const*)(uintptr_t)Cmd->DataPtr, memcpy_size);
	}
	else
	{
		Payload = (void *)(BlockwriteBuffer + (Hdr->Size / 4));
		memcpy((void*)Payload, (void const*)(uintptr_t)Cmd->DataPtr, memcpy_size);
		Hdr->Size +=Cmd->Size * (u32)sizeof(u32);
	}
}

static inline u32 _XAie_AppendBWToTxnBuff_opt(u32* BlockwriteBuffer,u8* TxnPtr, u32 PatchCmdCount)
{
	XAie_BlockWrite32Hdr_opt *Hdr = (XAie_BlockWrite32Hdr_opt*)(uintptr_t)BlockwriteBuffer;
	u32 Size = 0,PatchCmdSize=0;
	u8* TempPtr = NULL;

	if(PatchCmdCount != 0){

		PatchCmdSize = ((PatchCmdCount) * ( sizeof(patch_op_opt_t) + sizeof(XAie_CustomOpHdr_opt) )) & UINT_MAX;
		XAIE_DBG("_XAie_AppendBWToTxnBuff:PatchCmdSize = %d\n", PatchCmdSize);

		TempPtr = calloc(1, PatchCmdSize);
		if(TempPtr == NULL) {
			XAIE_ERROR("Calloc failed\n");
			return 0;
		}

		TxnPtr -= PatchCmdSize;
		memcpy(TempPtr, TxnPtr, PatchCmdSize);
		memset(TxnPtr, 0, PatchCmdSize);
	}
	else
	{
		XAIE_DBG("PatchCmdCount is 0\n");
	}

	memcpy((void*)TxnPtr, (const void *)(uintptr_t)BlockwriteBuffer, Hdr->Size);
	Size = Hdr->Size;
	memset(BlockwriteBuffer, 0, Size);

	if(PatchCmdCount != 0)
	{
		TxnPtr += Size;
		memcpy(TxnPtr, TempPtr, PatchCmdSize);
		free(TempPtr);
	}
	else
	{
		XAIE_DBG("PatchCmdCount is 0\n");
	}

    return Size;
}

u8* _XAie_TxnExportSerialized_opt(XAie_DevInst *DevInst, u8 NumConsumers,
		u32 Flags)
{
	const XAie_Backend *Backend = DevInst->Backend;
	XAie_TxnInst *TmpInst = NULL;
	u8 *TxnPtr = NULL;
	u8* LoadSeqCountPtr = NULL;
	u32 LoadSeqCountOffset = 0;
	u32 LoadSeqCount = 0;
	u32 BuffSize = 0U, NumOps = 0, PatchCmdCount = 0, BWBuffSize = 0;
	u64 RegOffLastBlockWrite = 0;
	u8 FirstBlockwriteProcessed = 0;
	u32* BlockwriteBuffer = NULL;
	u32 AllocatedBuffSize = XAIE_DEFAULT_TXN_BUFFER_SIZE;
	u32 BWBuffAllocatedSize = XAIE_DEFAULT_TXN_BUFFER_SIZE;
	(void)NumConsumers;
	(void)Flags;

	TmpInst = _XAie_GetTxnInst(DevInst, Backend->Ops.GetTid());
	if(TmpInst == NULL) {
		XAIE_ERROR("Failed to get the correct transaction instance "
				"from internal list\n");
		return NULL;
	}

	BlockwriteBuffer = calloc(1, BWBuffAllocatedSize);
	if(BlockwriteBuffer == NULL) {
		XAIE_ERROR("BlockWrite Buffer Calloc failed\n");
		return NULL;
	}

	TxnPtr = calloc(1, AllocatedBuffSize);
	if(TxnPtr == NULL) {
		XAIE_ERROR("TxnPtr Calloc failed\n");
		free(BlockwriteBuffer);
		return NULL;
	}

	_XAie_CreateTxnHeader_opt(DevInst, (XAie_TxnHeader*)(uintptr_t)TxnPtr);
	BuffSize += (u32)sizeof(XAie_TxnHeader);
	TxnPtr += sizeof(XAie_TxnHeader);
	XAIE_DBG("number of cmd %d\n", TmpInst->NumCmds);

	for(u32 i = 0U; i < TmpInst->NumCmds; i++) {
		NumOps++;
		XAie_TxnCmd *Cmd = &TmpInst->CmdBuf[i];

		if( (DevInst->PmLoadingActive == 1) && (Cmd->Opcode != XAIE_IO_LOAD_PM_START) && (Cmd->Opcode != XAIE_IO_LOAD_PM_END_INTERNAL))
		{
			LoadSeqCount++;
		}

		if( (Cmd->Opcode != XAIE_IO_BLOCKWRITE) &&
		    (Cmd->Opcode != XAIE_IO_CUSTOM_OP_DDR_PATCH) &&
		    (FirstBlockwriteProcessed != 0) )
		{
			XAie_BlockWrite32Hdr_opt *Hdr = (XAie_BlockWrite32Hdr_opt*)(uintptr_t)BlockwriteBuffer;
			while((BuffSize + Hdr->Size) > AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
				(AllocatedBuffSize) * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			BuffSize += Hdr->Size;
			TxnPtr += _XAie_AppendBWToTxnBuff_opt(BlockwriteBuffer,TxnPtr,PatchCmdCount);
			PatchCmdCount = 0;
			FirstBlockwriteProcessed = 0;
		}
	
		if ((Cmd->Opcode == XAIE_IO_WRITE) && (Cmd->Mask == 0U)) {
			while((BuffSize + sizeof(XAie_Write32Hdr_opt)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL){
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendWrite32_opt(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_Write32Hdr_opt);
			BuffSize += (u32)sizeof(XAie_Write32Hdr_opt);
			continue;
		}
		else if ((Cmd->Opcode == XAIE_IO_WRITE) && ((Cmd->Mask)!=0U)) {
			while((BuffSize + sizeof(XAie_MaskWrite32Hdr_opt)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendMaskWrite32_opt(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_MaskWrite32Hdr_opt);
			BuffSize += (u32)sizeof(XAie_MaskWrite32Hdr_opt);
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_MASKPOLL) {
			while((BuffSize + sizeof(XAie_MaskPoll32Hdr_opt)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendMaskPoll32_opt(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_MaskPoll32Hdr_opt);
			BuffSize += (u32)sizeof(XAie_MaskPoll32Hdr_opt);
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_MASKPOLL_BUSY) {
			while((BuffSize + sizeof(XAie_MaskPoll32Hdr_opt)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendMaskPollBusy32_opt(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_MaskPoll32Hdr_opt);
			BuffSize += (u32)sizeof(XAie_MaskPoll32Hdr_opt);
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_BLOCKWRITE) {
			/**
			 * In case of Block Write and Block Set, it is possible
			 * that the new allocated buffer size may not be sufficient.
			 * In that case we should keep reallocating till the new
			 * buffer size if big enough to hold existing + current opcode.
			 */
			BWBuffSize = ((XAie_BlockWrite32Hdr_opt*)(uintptr_t)BlockwriteBuffer)->Size;
			if(FirstBlockwriteProcessed != 0)
			{
			 	if ( Cmd->RegOff != RegOffLastBlockWrite)
			 	{
					while((BuffSize + BWBuffSize) > AllocatedBuffSize) {
						TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						(AllocatedBuffSize) * 2U, BuffSize);
						if(TxnPtr == NULL) {
							XAIE_ERROR("TxnPtr realloc failed\n");
							free(BlockwriteBuffer);
							return NULL;
						}
						AllocatedBuffSize *= 2U;
						TxnPtr += BuffSize;
					}
					BuffSize += BWBuffSize;
					TxnPtr += _XAie_AppendBWToTxnBuff_opt(BlockwriteBuffer,TxnPtr,PatchCmdCount);
					PatchCmdCount = 0;
					FirstBlockwriteProcessed = 0;
				}
				else
				{
					NumOps--;

					/**
					 * Load Sequence Count should be reduced only if there is an active
					 * PM Loading else do nothing.
					 **/
					if (DevInst->PmLoadingActive) {
						LoadSeqCount--;
					}
				}
			} else {
				/**
				 *  When BW Buffer is empty which is represented by FirstBlockwriteProcessed == 0
				 *  BWBuffSize should be initialized to size of XAie_BlockWrite32Hdr_opt struct in bytes.
				 *  Since the Cmd->Size field of Cmd from TmpInst->CmdBuf only considers payload size.
				 */
				BWBuffSize = sizeof(XAie_BlockWrite32Hdr_opt);
			}

			while( (Cmd->Size * 4) + BWBuffSize > BWBuffAllocatedSize)
			{
				BlockwriteBuffer = (u32*) (uintptr_t) ( _XAie_ReallocTxnBuf_MemInit((u8 *)BlockwriteBuffer,
									( BWBuffAllocatedSize + BWBuffAllocatedSize), BWBuffSize) );
				if(BlockwriteBuffer == NULL) {
					XAIE_ERROR("BlockWrite Buffer Realloc Failed\n");
					// To free Txn Buffer successfully we need to use the start pointer not current pointer
					free(TxnPtr - BuffSize);
					return NULL;
				}
				 BWBuffAllocatedSize *= 2U;
			}
			RegOffLastBlockWrite = (u64) ( Cmd->RegOff + (Cmd->Size*4) );
			_XAie_AppendBWToBlockwriteBuff_opt(Cmd, FirstBlockwriteProcessed, BlockwriteBuffer);
			FirstBlockwriteProcessed = 1;
		}
		else if (Cmd->Opcode == XAIE_IO_BLOCKSET) {
			/*
			 * Blockset gets converted to blockwrite. so check for
			 * blockwrite size
			 */
			while((BuffSize + sizeof(XAie_BlockWrite32Hdr_opt) +
					Cmd->Size * sizeof(u32)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendBlockSet32_opt(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_BlockWrite32Hdr_opt) +
				Cmd->Size * sizeof(u32);
			BuffSize += (u32)sizeof(XAie_BlockWrite32Hdr_opt) +
				Cmd->Size * (u32)sizeof(u32);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_NOOP)
		{
			while( (BuffSize + sizeof(XAie_NoOpHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendNoOp(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_NoOpHdr);
			BuffSize += (u32)sizeof(XAie_NoOpHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_PREEMPT)
		{
			while( (BuffSize + sizeof(XAie_PreemptHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendPreempt(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_PreemptHdr);
			BuffSize += (u32)sizeof(XAie_PreemptHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_LOADPDI)
		{
			while( (BuffSize + sizeof(XAie_LoadPdiHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendLoadPdi(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_LoadPdiHdr);
			BuffSize += (u32)sizeof(XAie_LoadPdiHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_LOAD_PM_START)
		{
			while( (BuffSize + sizeof(XAie_PmLoadHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			LoadSeqCountPtr = _XAie_AppendPmLoad(Cmd, TxnPtr);
			LoadSeqCountOffset = LoadSeqCountPtr - (TxnPtr - BuffSize);
			TxnPtr += sizeof(XAie_PmLoadHdr);
			BuffSize += (u32)sizeof(XAie_PmLoadHdr);
			DevInst->PmLoadingActive = 1;
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_CREATE_SCRATCHPAD)
		{
			while( (BuffSize + sizeof(XAie_CreateScratchpadHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendCreateScratchpad(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_CreateScratchpadHdr);
			BuffSize += (u32)sizeof(XAie_CreateScratchpadHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_UPDATE_STATE_TABLE)
		{
			while((BuffSize + sizeof(XAie_UpdateStateHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendUpdateState(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_UpdateStateHdr);
			BuffSize += (u32)sizeof(XAie_UpdateStateHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_UPDATE_REG)
		{
			/* Check for 16KB boundary crossing and add padding if needed */
			TxnPtr = _XAie_HandleTxnCmd16KBBoundary(TxnPtr, &BuffSize,
								&AllocatedBuffSize, (u32)sizeof(XAie_UpdateRegHdr),
								BlockwriteBuffer, &NumOps);
			if (TxnPtr == NULL) {
				return NULL;
			}
			while((BuffSize + sizeof(XAie_UpdateRegHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendUpdateReg(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_UpdateRegHdr);
			BuffSize += (u32)sizeof(XAie_UpdateRegHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_UPDATE_SCRATCH)
		{
			while( (BuffSize + sizeof(XAie_UpdateScratchHdr)) >
					AllocatedBuffSize ) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendUpdateScratch(Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_UpdateScratchHdr);
			BuffSize += (u32)sizeof(XAie_UpdateScratchHdr);
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_LOAD_PM_END_INTERNAL)
		{
			// Check if LoadSeqCountPtr needs to be updated due to realloc of TXN Ptr.
			if ((LoadSeqCountOffset != 0) && (LoadSeqCountPtr != ((TxnPtr - BuffSize) + LoadSeqCountOffset))) {
				LoadSeqCountPtr = (TxnPtr - BuffSize) + LoadSeqCountOffset;
			}

			if ((LoadSeqCount != 0) && (LoadSeqCountPtr != NULL))
			{
				*LoadSeqCountPtr = LoadSeqCount & 0xFF;
				*(LoadSeqCountPtr + 1) = (LoadSeqCount & 0xFF00) >> 8;
				*(LoadSeqCountPtr + 2) = (LoadSeqCount & 0xFF0000) >> 16;
			}
			else if (LoadSeqCountPtr == NULL)
			{
				XAIE_ERROR("LoadSeqCountPtr is equal to NULL\n");
				free(BlockwriteBuffer);
				// To free Txn Buffer successfully we need to use the start pointer not current pointer
				free(TxnPtr - BuffSize);
				return NULL;
			}
			LoadSeqCount = 0;
			NumOps--;
			DevInst->PmLoadingActive = 0;
		}
		else if (Cmd->Opcode >= XAIE_IO_CUSTOM_OP_TCT) {

			if(Cmd->Opcode == XAIE_IO_CUSTOM_OP_DDR_PATCH)
			{
				++PatchCmdCount;
			}
			if (TX_DUMP_ENABLE) {
				TxnCmdDump(Cmd);
			}

			XAIE_DBG("Size of the CustomOp Hdr being exported: %u bytes\n", sizeof(XAie_CustomOpHdr));

			while((BuffSize + sizeof(XAie_CustomOpHdr_opt) +
						Cmd->Size) > AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U, BuffSize);
				if(TxnPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			
			if(Cmd->Opcode != XAIE_IO_CUSTOM_OP_DDR_PATCH){
				_XAie_AppendCustomOp_opt(Cmd, TxnPtr);
			}
			else{
				_XAie_AppendDDRPatch_opt(Cmd, TxnPtr);
			}
			TxnPtr += sizeof(XAie_CustomOpHdr_opt) +
				Cmd->Size * sizeof(u8);
			BuffSize += (u32)sizeof(XAie_CustomOpHdr_opt) +
				Cmd->Size * (u32)sizeof(u8);
			continue;
		}
	}
	if(FirstBlockwriteProcessed != 0)
	{	
		XAie_BlockWrite32Hdr_opt *Hdr = (XAie_BlockWrite32Hdr_opt*)(uintptr_t)BlockwriteBuffer;
		while((BuffSize + Hdr->Size) > AllocatedBuffSize) {
			TxnPtr = _XAie_ReallocTxnBuf_MemInit(TxnPtr - BuffSize,
						(AllocatedBuffSize) * 2U, BuffSize);
			if(TxnPtr == NULL) {
				XAIE_ERROR("TxnPtr realloc failed\n");
				free(BlockwriteBuffer);
				return NULL;
			}
			AllocatedBuffSize *= 2U;
			TxnPtr += BuffSize;
		}
		BuffSize += Hdr->Size;
		TxnPtr += _XAie_AppendBWToTxnBuff_opt(BlockwriteBuffer,TxnPtr,PatchCmdCount);
	}

	// Free the BlockwriteBuffer
	free(BlockwriteBuffer);
    
	u32 four_byte_aligned_BuffSize = ((BuffSize % 4U) != 0U) ? ((BuffSize / 4U + 1U)*4) : BuffSize;
	XAIE_DBG("Size of the Txn Hdr being exported: %u bytes\n",
			sizeof(XAie_TxnHeader));
	XAIE_DBG("Size of the transaction buffer being exported: %u bytes\n",
			four_byte_aligned_BuffSize);
	XAIE_DBG("Num of Operations in the transaction buffer: %u\n",
			TmpInst->NumCmds);

	/* Adjust pointer and reallocate to the right size */
	TxnPtr = _XAie_ReallocTxnBuf(TxnPtr - BuffSize, four_byte_aligned_BuffSize);
	if(TxnPtr == NULL) {
		XAIE_ERROR("TxnPtr realloc failed\n");
		return NULL;
	}
	
	((XAie_TxnHeader *)(uintptr_t)TxnPtr)->NumOps =  NumOps;
	((XAie_TxnHeader *)(uintptr_t)TxnPtr)->TxnSize =  four_byte_aligned_BuffSize;
	return (u8 *)TxnPtr;
}

void _XAie_FreeTxnPtr(void *Ptr)
{
	free(Ptr);
}

/*****************************************************************************/
/**
*
* This api releases the memory resources used by exported transaction instance.
*
* @param	TxnInst - Existing Transaction instance
*
* @return	XAIE_OK on success or error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_TxnFree(XAie_TxnInst *Inst)
{
	if((Inst->Flags & XAIE_TXN_INST_EXPORTED_MASK) == 0U) {
		XAIE_ERROR("The transaction instance was not exported, it's "
				"resources cannot be released\n");
		return XAIE_ERR;
	}

	for(u32 i = 0; i < Inst->NumCmds; i++) {
		XAie_TxnCmd *Cmd = &Inst->CmdBuf[i];

		if((Cmd->Opcode == XAIE_IO_BLOCKWRITE ||
			(Cmd->Opcode >= XAIE_IO_CUSTOM_OP_BEGIN && Cmd->Opcode < XAIE_IO_CUSTOM_OP_NEXT)) &&
			((void *)(uintptr_t)Cmd->DataPtr != NULL)) {
			free((void *)(uintptr_t)Cmd->DataPtr);
		}
	}

	free(Inst->CmdBuf);
	free(Inst);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This api cleansup all the resources when the device instance is closed.
*
* @param	DevInst - Device instance pointer.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
void _XAie_TxnResourceCleanup(XAie_DevInst *DevInst)
{
	XAie_List *NodePtr = DevInst->TxnList.Next;
	XAie_TxnInst *TxnInst;

	while(NodePtr != NULL) {
		TxnInst = (XAie_TxnInst *)(uintptr_t)XAIE_CONTAINER_OF(NodePtr, XAie_TxnInst, Node);

		if (TxnInst == NULL) {
			 continue;
		}

		for(u32 i = 0; i < TxnInst->NumCmds; i++) {
			XAie_TxnCmd *Cmd = &TxnInst->CmdBuf[i];

			if((Cmd->Opcode == XAIE_IO_BLOCKWRITE ||
				(Cmd->Opcode >= XAIE_IO_CUSTOM_OP_BEGIN && Cmd->Opcode < XAIE_IO_CUSTOM_OP_NEXT)) &&
				((void *)(uintptr_t)Cmd->DataPtr != NULL)) {
				free((void *)(uintptr_t)Cmd->DataPtr);
			}
		}

		NodePtr = NodePtr->Next;
		free(TxnInst->CmdBuf);
		free(TxnInst);
	}
}

/*****************************************************************************/
/**
*
* This API request a custom operation code that can be added to the transaction buffer.
*
* @param    DevInst - Global AIE device instance pointer.
*
* @return   Enum of the custom operation number.
*
* @note     This function may be called before or after the XAie_StartTransaction();
*
******************************************************************************/

int XAie_RequestCustomTxnOp(XAie_DevInst *DevInst) {

	XAie_TxnInst *Inst;
	const XAie_Backend *Backend = DevInst->Backend;

	Inst = _XAie_GetTxnInst(DevInst, Backend->Ops.GetTid());
	if(Inst == NULL) {
		XAIE_ERROR("Failed to get the correct transaction instance "
				"from internal list\n");
		return (int)XAIE_ERR;
	}
	if(Inst->NextCustomOp < (u8)XAIE_IO_CUSTOM_OP_MAX) {
		XAIE_DBG("New Custom OP allocated %d\n", Inst->NextCustomOp);
	} else {
		XAIE_ERROR("Custom OP Max reached %d, allocation fail\n", Inst->NextCustomOp);
		return -1;
	}

	return (int)(Inst->NextCustomOp)++;
}

/*****************************************************************************/
/**
*
* This API registers a custom operation that can be added to the transaction buffer.
*
* @param    DevInst - Global AIE device instance pointer.
* @param    OpNum - OpNum returned by the AIE driver when the custom op was registered.
* @param    Args - Args required by the custom Op.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_AddCustomTxnOp(XAie_DevInst *DevInst, u8 OpNumber, void* Args, size_t size) {

	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance "
					"associated with thread. Polling "
					"from register\n");
			return XAIE_ERR;
		}

		if ( TxnInst->NextCustomOp < OpNumber || (u8)XAIE_IO_CUSTOM_OP_BEGIN > OpNumber ) {
			XAIE_ERROR("Invalid Op Code %d\n",OpNumber);
			return XAIE_ERR;
		}

		/* check memory allocation before increase Cmd vector */
		char* tmpBuff = malloc(size);
		if(tmpBuff == NULL) {
			XAIE_ERROR("Fail to malloc %lu size memory for DataPtr\n", size);
			return XAIE_ERR;
		}

		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if (RC != XAIE_OK) {
				free(tmpBuff);
				return RC;
			}

		}

		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = (XAie_TxnOpcode)OpNumber;
		TxnInst->CmdBuf[TxnInst->NumCmds].Size = (u32)size;

		memcpy(tmpBuff, Args, size);
		TxnInst->CmdBuf[TxnInst->NumCmds].DataPtr = (u64)tmpBuff;

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_WRITE that can be added to the transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    RegOff - Register Offset. 
* @param    Value - Value to be written to the register.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_Write32(XAie_DevInst *DevInst, u64 RegOff, u32 Value) {
	u64 Tid;
	AieRC RC;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	Tid = Backend->Ops.GetTid();
	TxnInst = _XAie_GetTxnInst(DevInst, Tid);
	if(TxnInst == NULL) {
		XAIE_DBG("Could not find transaction instance "
				"associated with thread. Mask writing "
				"to register\n");
		return Backend->Ops.Write32((void*)(DevInst->IOInst), RegOff, Value);
	}

	if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
		RC = _XAie_ReallocCmdBuf(TxnInst);
		if (RC != XAIE_OK) {
			return RC;
		}
	}

	TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_WRITE;
	TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
	TxnInst->CmdBuf[TxnInst->NumCmds].Value = Value;
	TxnInst->CmdBuf[TxnInst->NumCmds].Mask = 0U;
	TxnInst->NumCmds++;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API calls Backend Read32 to read data from given register.
* @param    DevInst - Global AIE device instance pointer.
* @param    RegOff - Register Offset. 
* @param    Data - Pointer to data address to be read from the register.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_Read32(XAie_DevInst *DevInst, u64 RegOff, u32 *Data) {
	u64 Tid;
	AieRC RC;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	Tid = Backend->Ops.GetTid();
	TxnInst = _XAie_GetTxnInst(DevInst, Tid);
	if(TxnInst == NULL) {
		XAIE_DBG("Could not find transaction instance "
				"associated with thread. Reading "
				"from register\n");
		return Backend->Ops.Read32((void*)(DevInst->IOInst), RegOff, Data);
	}

	if(((TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) != 0U) &&
			(TxnInst->NumCmds > 0U)) {
		/* Flush command buffer */
		XAIE_DBG("Auto flushing contents of the transaction "
				"buffer.\n");
		RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to flush cmd buffer\n");
			return RC;
		}
		TxnInst->NumCmds = 0;

		return Backend->Ops.Read32((void*)(DevInst->IOInst), RegOff, Data);
	} else if(TxnInst->NumCmds == 0U) {
		return Backend->Ops.Read32((void*)(DevInst->IOInst), RegOff, Data);
	} else {
		XAIE_ERROR("Read operation is not supported "
				"when auto flush is disabled\n");
		return XAIE_ERR;
	}
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_WRITE that can be added to the transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    RegOff - Register Offset.
* @param    Mask - Mask to write specific bits into register. 
* @param    Value - Value to be written to the register.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_MaskWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value) {
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	Tid = Backend->Ops.GetTid();
	TxnInst = _XAie_GetTxnInst(DevInst, Tid);
	if(TxnInst == NULL) {
		XAIE_DBG("Could not find transaction instance "
				"associated with thread. Writing "
				"to register\n");
		return Backend->Ops.MaskWrite32((void *)(DevInst->IOInst), RegOff, Mask,
				Value);
	}

	if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
		RC = _XAie_ReallocCmdBuf(TxnInst);
		if (RC != XAIE_OK) {
				return RC;
		}

	}
	TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_WRITE;
	TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
	TxnInst->CmdBuf[TxnInst->NumCmds].Mask = Mask;
	TxnInst->CmdBuf[TxnInst->NumCmds].Value = Value;
	TxnInst->NumCmds++;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_MASKPOLL that can be added to the transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    RegOff - Register Offset.
* @param    Mask - Mask to poll on specific bits. 
* @param    Value - Expected desired value in register.
* @param    TimeOutUs - Maximum polling time on register to become desired value.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_MaskPoll(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value, u32 TimeOutUs) {
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	Tid = Backend->Ops.GetTid();
	TxnInst = _XAie_GetTxnInst(DevInst, Tid);
	if(TxnInst == NULL) {
		XAIE_DBG("Could not find transaction instance "
				"associated with thread. Polling "
				"from register\n");
		return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
				Value, TimeOutUs);
	}

	if(((TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) != 0U) &&
			(TxnInst->NumCmds > 0U)) {
		/* Flush command buffer */
		XAIE_DBG("Auto flushing contents of the transaction "
				"buffer.\n");
		RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to flush cmd buffer\n");
			return RC;
		}

		TxnInst->NumCmds = 0;
		return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
				Value, TimeOutUs);
	} else {
		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if (RC != XAIE_OK) {
					return RC;
			}

		}
		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_MASKPOLL;
		TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
		TxnInst->CmdBuf[TxnInst->NumCmds].Mask = Mask;
		TxnInst->CmdBuf[TxnInst->NumCmds].Value = Value;
		TxnInst->NumCmds++;

		return XAIE_OK;
	}
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_MASKPOLL_BUSY that can be added to the transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    RegOff - Register Offset. 
* @param    Mask - Mask to poll on specific bits. 
* @param    Value - Expected desired value in register.
* @param    TimeOutUs - Maximum polling time on register to become desired value.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_MaskPollBusy(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value, u32 TimeOutUs) {
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	Tid = Backend->Ops.GetTid();
	TxnInst = _XAie_GetTxnInst(DevInst, Tid);
	if(TxnInst == NULL) {
		XAIE_DBG("Could not find transaction instance "
				"associated with thread. Polling "
				"from register\n");
		return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
				Value, TimeOutUs);
	}

	if(((TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) != 0U) &&
			(TxnInst->NumCmds > 0U)) {
		/* Flush command buffer */
		XAIE_DBG("Auto flushing contents of the transaction "
				"buffer.\n");
		RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to flush cmd buffer\n");
			return RC;
		}

		TxnInst->NumCmds = 0;
		return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
				Value, TimeOutUs);
	} else {
		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if (RC != XAIE_OK) {
					return RC;
			}
		}
		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_MASKPOLL_BUSY;
		TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
		TxnInst->CmdBuf[TxnInst->NumCmds].Mask = Mask;
		TxnInst->CmdBuf[TxnInst->NumCmds].Value = Value;
		TxnInst->NumCmds++;

		return XAIE_OK;
	}
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_BLOCKWRITE that can be added to the transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    RegOff - Register Offset. 
* @param    Data - Pointer to Data to be written into the registers.
* @param    Size - Size of the data to be written to the registers in bytes.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_BlockWrite32(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data, u32 Size) {
	AieRC RC;
	u64 Tid;
	u32 *Buf;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	Tid = Backend->Ops.GetTid();
	TxnInst = _XAie_GetTxnInst(DevInst, Tid);
	if(TxnInst == NULL) {
		XAIE_DBG("Could not find transaction instance "
				"associated with thread. Block write "
				"to register\n");
		return Backend->Ops.BlockWrite32((void *)(DevInst->IOInst), RegOff,
				Data, Size);
	}

	if(TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) {
		/* Flush command buffer */
		XAIE_DBG("Auto flushing contents of the transaction "
				"buffer.\n");
		if(TxnInst->NumCmds > 0U) {
			RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
			if (RC != XAIE_OK) {
				XAIE_ERROR("Failed to flush cmd buffer\n");
				return RC;
			}
		}

		TxnInst->NumCmds = 0;
		return Backend->Ops.BlockWrite32((void *)(DevInst->IOInst), RegOff,
				Data, Size);
	}

	if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
		RC = _XAie_ReallocCmdBuf(TxnInst);
		if (RC != XAIE_OK) {
			return RC;
		}
	}

	Buf = (u32 *)malloc(sizeof(u32) * Size);
	if(Buf == NULL) {
		XAIE_ERROR("Memory allocation for block write failed\n");
		return XAIE_ERR;
	}

	Buf = memcpy((void *)Buf, (const void *)Data, sizeof(u32) * Size);
	TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_BLOCKWRITE;
	TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
	TxnInst->CmdBuf[TxnInst->NumCmds].DataPtr = (u64)(uintptr_t)Buf;
	TxnInst->CmdBuf[TxnInst->NumCmds].Size = Size;
	TxnInst->CmdBuf[TxnInst->NumCmds].Mask = 0U;
	TxnInst->NumCmds++;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_BLOCKSET that can be added to the transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    RegOff - Register Offset. 
* @param    Data - Data to be written to the register.
* @param    Size - Size of the data to be written to the register in bytes.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_BlockSet32(XAie_DevInst *DevInst, u64 RegOff, const u32 Data, u32 Size) {
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	Tid = Backend->Ops.GetTid();
	TxnInst = _XAie_GetTxnInst(DevInst, Tid);
	if(TxnInst == NULL) {
		XAIE_DBG("Could not find transaction instance "
				"associated with thread. Block set "
				"to register\n");
		return Backend->Ops.BlockSet32((void *)(DevInst->IOInst), RegOff, Data,
				Size);
	}

	if(TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) {
		/* Flush command buffer */
		XAIE_DBG("Auto flushing contents of the transaction "
				"buffer.\n");
		if(TxnInst->NumCmds > 0U) {
			RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to flush cmd buffer\n");
				return RC;
			}
		}

		TxnInst->NumCmds = 0;
		return Backend->Ops.BlockSet32((void *)(DevInst->IOInst), RegOff, Data,
				Size);
	}

	if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
		RC = _XAie_ReallocCmdBuf(TxnInst);
		if (RC != XAIE_OK) {
			return RC;
		}
	}

	TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_BLOCKSET;
	TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
	TxnInst->CmdBuf[TxnInst->NumCmds].Value = Data;
	TxnInst->CmdBuf[TxnInst->NumCmds].Size = Size;
	TxnInst->CmdBuf[TxnInst->NumCmds].Mask = 0U;
	TxnInst->NumCmds++;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API register NO_OP that can be added to the transaction buffer count times.
* @param    DevInst - Global AIE device instance pointer. 
* @param    Count - Number of time NO-OP needs to be added to transaction buffer
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_NoOp(XAie_DevInst *DevInst, uint32_t Count)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) 
	{
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance "
					"associated with thread. Polling "
					"from register\n");
			return XAIE_ERR;
		}
		for( uint32_t loop=0; loop<Count; loop++)
		{

			if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
					RC = _XAie_ReallocCmdBuf(TxnInst);
					if (RC != XAIE_OK) {
				 		return RC;
					}

			}
			TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_NOOP;

			if (TX_DUMP_ENABLE) {
				TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
			}

			TxnInst->NumCmds++;
		}

		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_PREEMPT that can be added to the transaction buffer.
* @param    DevInst - Global AIE device instance pointer. 
* @param    Preempt - Preempt struct that needs to be filled in by the caller
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_Preempt(XAie_DevInst *DevInst, XAie_PreemptHdr* Preempt)
{
	u8 p_level = Preempt->Preempt_level;
	if(p_level >= INVALID)
	{
		XAIE_ERROR("Error: Preempt_level = %d",p_level);
		return XAIE_ERR;
	}

	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) 
	{
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance "
					"associated with thread. Polling "
					"from register\n");
			return XAIE_ERR;
		}
		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
				RC = _XAie_ReallocCmdBuf(TxnInst);
				if (RC != XAIE_OK) {
				 	return RC;
				}

		}
		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_PREEMPT;
		TxnInst->CmdBuf[TxnInst->NumCmds].Preempt_level = p_level;

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API registers XAIE_IO_LOADPDI that can be added to the transaction buffer.
* @param    DevInst - Global AIE device instance pointer. 
* @param    PdiId - Unique id of the pdi.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_LoadPdi(XAie_DevInst *DevInst, u16 PdiId)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) 
	{
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance associated with thread. Polling from register\n");
			return XAIE_ERR;
		}
		if( (TxnInst->NumCmds + 1U) == TxnInst->MaxCmds) {
				RC = _XAie_ReallocCmdBuf(TxnInst);
				if (RC != XAIE_OK) {
				 	return RC;
				}

		}
		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_LOADPDI;
		TxnInst->CmdBuf[TxnInst->NumCmds].PdiId = PdiId;

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_CUSTOM_OP_MERGE_SYNC that can be added to the
* transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    Preempt - Preempt struct that needs to be filled in by the caller
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_MergeSync(XAie_DevInst *DevInst, u8 num_tokens, u8 num_cols)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if (num_cols > DevInst->NumCols) {
		XAIE_ERROR("Invalid argument num_cols > max columns reserved for partition\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->TxnList.Next != NULL)
	{
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance "
					"associated with thread. Polling "
					"from register\n");
			return XAIE_ERR;
		}
		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if (RC != XAIE_OK) {
				return RC;
			}
		}

		u32* tctDataBuff = (u32 *)calloc(1,sizeof(tct_op_t));
		if(tctDataBuff != NULL) {
			tctDataBuff[0] = ((0x000000FF & num_tokens) | (0x0000FF00 & (num_cols << 8)));
			TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_CUSTOM_OP_MERGE_SYNC;
			TxnInst->CmdBuf[TxnInst->NumCmds].Size = (u32)sizeof(tct_op_t);
			TxnInst->CmdBuf[TxnInst->NumCmds].DataPtr = (u64)tctDataBuff;
		}
		else
		{
			XAIE_ERROR("tctDataBuff is NULL\n");
			return XAIE_ERR;
		}

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_CUSTOM_OP_DDR_PATCH that can be added to the
* transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    Preempt - Preempt struct that needs to be filled in by the caller
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_DdrAddressPatch(XAie_DevInst *DevInst, u64 regaddr, u64 argidx,
                               u64 argplus)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL)
	{
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance "
					"associated with thread. Polling "
					"from register\n");
			return XAIE_ERR;
		}
		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
				RC = _XAie_ReallocCmdBuf(TxnInst);
				if (RC != XAIE_OK) {
					return RC;
				}
		}

		patch_op_t* patchDataBuff = (patch_op_t *)calloc(1,sizeof(patch_op_t));
		if(patchDataBuff != NULL) {
			patchDataBuff->regaddr = regaddr;
			patchDataBuff->argidx = argidx;
			patchDataBuff->argplus = argplus;
			TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_CUSTOM_OP_DDR_PATCH;
			TxnInst->CmdBuf[TxnInst->NumCmds].Size = (u32)sizeof(patch_op_t);
			TxnInst->CmdBuf[TxnInst->NumCmds].DataPtr = (u64)patchDataBuff;
		}
		else
		{
			XAIE_ERROR("patchDataBuff is NULL\n");
			return XAIE_ERR;
		}

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_LOAD_PM_START that can be added to the
* transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    PmLoadId - Unique Id of PM.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_PmLoadStart(XAie_DevInst *DevInst, u32 PmLoadId)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL)
	{
		if(DevInst->PmLoadingActive == 1)
		{
			XAIE_ERROR("Back to back XAie_Txn_PmLoadStart API calls, without calling XAie_Txn_PmLoadEnd for the fist start\n");
			return XAIE_ERR;
		}
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance "
					"associated with thread. Polling "
					"from register\n");
			return XAIE_ERR;
		}
		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
				RC = _XAie_ReallocCmdBuf(TxnInst);
				if (RC != XAIE_OK) {
					return RC;
				}
		}
		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_LOAD_PM_START;
		TxnInst->CmdBuf[TxnInst->NumCmds].PmId = PmLoadId;

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		XAIE_DBG("PM Loading using Txn flow Started\n");
		DevInst->PmLoadingActive = 1;

		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API marks end of sequence of PM loading operations corresponding to its PmLoadStart.
  This API should always be paired with XAie_Txn_PmLoadStart
*@param    DevInst - Global AIE device instance pointer.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_PmLoadEnd(XAie_DevInst *DevInst)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;
	if(DevInst->TxnList.Next != NULL)
	{
		if(DevInst->PmLoadingActive == 0)
		{
			XAIE_ERROR("XAie_Txn_PmLoadEnd called without its corresponding XAie_Txn_PmLoadStart\n");
			return XAIE_ERR;
		}
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance "
					"associated with thread. Polling "
					"from register\n");
			return XAIE_ERR;
		}
		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
				RC = _XAie_ReallocCmdBuf(TxnInst);
				if (RC != XAIE_OK) {
					return RC;
				}
		};
		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_LOAD_PM_END_INTERNAL;

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		XAIE_DBG("PM Loading using Txn flow Completed\n");
		DevInst->PmLoadingActive = 0;

		return XAIE_OK;
	}
	
	return XAIE_ERR;

}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_CREATE_SCRATCHPAD that can be added to the transaction buffer.
* @param    DevInst - Global AIE device instance pointer. 
* @param    UsageType - Scratchpad usage type
* @param    Size - Size of scratchpad in bytes.
* @param    ScratchOffset - Offset into the Scratchpad buffer on host side where the
*                           state table will be defined.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_CreateScratchpad(XAie_DevInst *DevInst, u8 UsageType, u32 Size,
								u64 ScratchOffset)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(UsageType != XAIE_STATE_TABLE)
	{
		XAIE_ERROR("Error: UsageType = %d (unsupported value)",UsageType);
		return XAIE_ERR;
	}

	if(DevInst->TxnList.Next != NULL)
	{
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance "
					"associated with thread. Polling "
					"from register\n");
			return XAIE_ERR;
		}
		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if (RC != XAIE_OK) {
				return RC;
			}
		}
		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_CREATE_SCRATCHPAD;
		TxnInst->CmdBuf[TxnInst->NumCmds].UsageType = UsageType;
		TxnInst->CmdBuf[TxnInst->NumCmds].Size = Size;
		TxnInst->CmdBuf[TxnInst->NumCmds].ScratchOffset = ScratchOffset;

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_UPDATESTATE TXN command that can be added to the
* transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    StateTableIdx - Index into the table.
* @param    Func - Operation to calculate new sate value (Mul/Incr/Decr).
* @param    FuncArg - Factor to be used for operation.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_UpdateStateTable(XAie_DevInst *DevInst, u8 StateTableIdx,
				XAie_StateTableFuncType Func, u32 FuncArg)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL)
	{
		if(StateTableIdx >= XAIE_TXN_STATE_TABLE_SIZE)
		{
			XAIE_ERROR("State Table Index exceeds the maximum size of 32\n");
			return XAIE_ERR;
		}
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance associated with thread. Polling from register\n");
			return XAIE_ERR;
		}
		if( (TxnInst->NumCmds + 1U) == TxnInst->MaxCmds) {
				RC = _XAie_ReallocCmdBuf(TxnInst);
				if (RC != XAIE_OK) {
					return RC;
				}
		}

		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_UPDATE_STATE_TABLE;
		TxnInst->CmdBuf[TxnInst->NumCmds].StateTableIdx = StateTableIdx;
		TxnInst->CmdBuf[TxnInst->NumCmds].Func = Func;
		TxnInst->CmdBuf[TxnInst->NumCmds].FuncArg = FuncArg;

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_UPDATEREG TXN command that can be added to the
* transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
* @param    StateTableIdx - Index into the table.
* @param    Func - Operation to calculate new sate value (Mul/Incr/Decr).
* @param    FuncArg - Factor to be used for operation.
* @param    RegOff - Register offset.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_UpdateReg(XAie_DevInst *DevInst, u64 RegOff, u8 StateTableIdx,
			 XAie_StateTableFuncType Func, u32 FuncArg)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL)
	{
		if(StateTableIdx >= XAIE_TXN_STATE_TABLE_SIZE)
		{
			XAIE_ERROR("State Table Index exceeds the maximum size of 32\n");
			return XAIE_ERR;
		}
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance associated with thread. Polling from register\n");
			return XAIE_ERR;
		}
		if( (TxnInst->NumCmds + 1U) == TxnInst->MaxCmds) {
				RC = _XAie_ReallocCmdBuf(TxnInst);
				if (RC != XAIE_OK) {
					return RC;
				}
		}

		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_UPDATE_REG;
		TxnInst->CmdBuf[TxnInst->NumCmds].StateTableIdx = StateTableIdx;
		TxnInst->CmdBuf[TxnInst->NumCmds].Func = Func;
		TxnInst->CmdBuf[TxnInst->NumCmds].FuncArg = FuncArg;
		TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API register XAIE_IO_UPDATE_SCRATCH that can be added to the transaction buffer.
* @param    DevInst - Global AIE device instance pointer.
*
* @return   XAIE_OK for success and error code otherwise.
*
* @note     This function must be called after XAie_StartTransaction();
*
******************************************************************************/
AieRC XAie_Txn_UpdateScratch(XAie_DevInst *DevInst)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL)
	{
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_ERROR("Could not find transaction instance "
					"associated with thread. Polling "
					"from register\n");
			return XAIE_ERR;
		}
		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if (RC != XAIE_OK) {
				return RC;
			}
		}

		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_UPDATE_SCRATCH;

		if (TX_DUMP_ENABLE) {
			TxnCmdDump(&TxnInst->CmdBuf[TxnInst->NumCmds]);
		}

		TxnInst->NumCmds++;

		return XAIE_OK;
	}

	return XAIE_ERR;

}

AieRC XAie_Txn_RunOp(XAie_DevInst *DevInst, u8 Is_Config_ShimDMABD, void *Arg)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	Tid = Backend->Ops.GetTid();
	TxnInst = _XAie_GetTxnInst(DevInst, Tid);
	if(TxnInst == NULL) {
		XAIE_DBG("Could not find transaction instance "
				"associated with thread. Running Op.\n");
		return XAIE_NOT_SUPPORTED;
	}

	if (Is_Config_ShimDMABD) {
		XAie_ShimDmaBdArgs *BdArgs =
			(XAie_ShimDmaBdArgs *)Arg;
			XAie_BlockWrite32(DevInst,
							BdArgs->Addr, BdArgs->BdWords,
							BdArgs->NumBdWords);
		return XAIE_OK;
	}

	if(((TxnInst->Flags & XAIE_TXN_AUTO_FLUSH_MASK) != 0U) &&
		(TxnInst->NumCmds > 0U)) {
		/* Flush command buffer */
		RC = _XAie_Txn_FlushCmdBuf(DevInst, TxnInst);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to flush cmd buffer\n");
			return RC;
		}
		TxnInst->NumCmds = 0;
		return XAIE_NOT_SUPPORTED;
	} else if(TxnInst->NumCmds == 0U){
		return XAIE_NOT_SUPPORTED;
	} else {
		XAIE_ERROR("Run Op operation is not supported "
				"when auto flush is disabled\n");
		return XAIE_ERR;
	}
}

/*****************************************************************************/
/**
*
* This should be the first API called by the user to configure the initial
* transaction command array size to be allocated dynamically. If this API is
* not called or called with 0 size then the default size of 1024 will be used.
*
* @param	DevInst: Device instance pointer.
* @param	CmdCount: Command count to be allocated.
*
* @return	XAIE_OK on success and error code on failure.
*
******************************************************************************/
AieRC XAie_CfgInitialTxnCmdArraySize(XAie_DevInst *DevInst, u32 CmdCount)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	return _XAie_Txn_CfgInitialCmdArraySize(DevInst, CmdCount);
}

/*****************************************************************************/
/**
*
* This API starts the execution of the driver in transaction mode. All the
* resulting I/O operations are stored in an internally managed buffer. The user
* has to explicitly submit the transaction for the driver to execute the I/O
* operations to configure the device. The transaction instance allocated by this
* API is tied to the thread ID of the executing context. SubmitTransaction API
* must be called from the same context with NULL for the TxnInst parameter.
*
* @param	DevInst: Device instance pointer.
* @param	Flags: Flags passed by the user.
*			XAIE_TRANSACTION_ENABLE/DISBALE_AUTO_FLUSH
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		If the ENABLE_AUTO_FLUSH flag is set, the driver will
*		automatically flush the transaction buffer when an API results
*		in Read/MaskPoll/BlockWrite/BlockSet/CmdWrite/RunOp operation.
*		If the DISABLE_AUTO_FLUSH flag is set, the driver will return an
*		error when an API results in Read/MaskPoll/CmdWrite/RunOp
*		operation. In both cases, the user has to call
*		XAie_SubmitTransaction API to flush all the pending I/O
*		operations stored in the command buffer.
*
******************************************************************************/
AieRC XAie_StartTransaction(XAie_DevInst *DevInst, u32 Flags)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	return _XAie_Txn_Start(DevInst, Flags);
}

/*****************************************************************************/
/**
*
* This API executes all the pending I/O operations stored in the command buffer.
* The transaction instance returned by the StartTransaction API is tied to the
* thread ID of the executing context. If TxnInst is NULL, the transaction
* instance is automatically fetched using the thread ID of the current context.
* If the TxnInst is not NULL, the transaction instance passed by the user is
* executed.
*
* @param	DevInst: Device instance pointer.
* @param	TxnInst: Transaction instance pointer.
*
* @return	XAIE_OK on success and Error code or failure.
*
******************************************************************************/
AieRC XAie_SubmitTransaction(XAie_DevInst *DevInst, XAie_TxnInst *TxnInst)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	return _XAie_Txn_Submit(DevInst, TxnInst);
}

/*****************************************************************************/
/**
*
* This API copies an existing transaction instance and returns a copy of the
* instance with all the commands for users to save the commands and use them
* at a later point.
*
* @param	DevInst: Device instance pointer.
*
* @return	Pointer to copy of transaction instance on success and NULL
*		on error.
*
* @note		The copy of the transaction instance must be explicitly freed
*		using the XAie_FreeTransactionInstance API. If Auto flush was
*		enabled during the creating of the initial transaction, the
*		instance returned by this API will not have the commands that
*		are already flushed. The transaction instance must be exported
*		before it is submitted as the XAie_SubmitTransaction API will
*		free all the resources associated with it.
*
******************************************************************************/
XAie_TxnInst* XAie_ExportTransactionInstance(XAie_DevInst *DevInst)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return NULL;
	}

	return _XAie_TxnExport(DevInst);
}

/*****************************************************************************/
/**
*
* This API copies an existing transaction instance and returns a copy of the
* instance with all the commands for users to save the commands and use them
* at a later point.
*
* @param	DevInst: Device instance pointer.
* @param	NumConsumers: Number of consumers for the generated
*		transactions (Unused for now)
* @param	Flags: Flags (Unused for now)
*
* @return	Pointer to copy of transaction instance on success and NULL
*		on error.
*
******************************************************************************/
u8* XAie_ExportSerializedTransaction(XAie_DevInst *DevInst,
		u8 NumConsumers, u32 Flags)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return NULL;
	}

	return _XAie_TxnExportSerialized(DevInst, NumConsumers, Flags);
}

/*****************************************************************************/
/**
*
* This API copies an existing transaction instance and returns a copy of the
* instance with all the commands for users to save the commands and use them
* at a later point.
*
* @param	DevInst: Device instance pointer.
* @param	NumConsumers: Number of consumers for the generated
*		transactions (Unused for now)
* @param	Flags: Flags (Unused for now)
*
* @return	Pointer to copy of transaction instance on success and NULL
*		on error.
*
******************************************************************************/
u8* XAie_ExportSerializedTransaction_opt(XAie_DevInst *DevInst,
		u8 NumConsumers, u32 Flags)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return NULL;
	}

    return _XAie_TxnExportSerialized_opt(DevInst, NumConsumers, Flags);
}


/*****************************************************************************/
/**
*
* This API releases the memory resources used by exported transaction instance.
*
* @param	TxnInst: Existing Transaction instance
*
* @return	XAIE_OK on success or error code on failure.
*
******************************************************************************/
AieRC XAie_FreeTransactionInstance(XAie_TxnInst *TxnInst)
{
	if(TxnInst == NULL) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	return _XAie_TxnFree(TxnInst);
}

/*****************************************************************************/
/**
*
* This API deallocates the memory allocated for the serialized transaction
* buffer.
*
* @param	Ptr: Pointer to the transaction buffer.
*
******************************************************************************/
void XAie_FreeSerializedTransaction(void *Ptr)
{
	if (Ptr == NULL) {
		XAIE_ERROR("Invalid argument\n");
		return;
	}
	_XAie_FreeTxnPtr(Ptr);
}

AieRC XAie_ClearTransaction(XAie_DevInst* DevInst)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	return _XAie_ClearTransaction(DevInst);
}

/** @} */
