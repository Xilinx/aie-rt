/******************************************************************************
* Copyright (C) 2019 - 2024 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_helper_internal.h
* @{
*
* This file contains inline helper functions for AIE drivers.
******************************************************************************/
#ifndef XAIE_HELPER_INTERNAL_H
#define XAIE_HELPER_INTERNAL_H

/***************************** Include Files *********************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
AieRC _XAie_GetSlaveIdx(const XAie_StrmMod *StrmMod, StrmSwPortType Slave,
		u8 PortNum, u8 *SlaveIdx);
AieRC _XAie_GetMstrIdx(const XAie_StrmMod *StrmMod, StrmSwPortType Master,
		u8 PortNum, u8 *MasterIdx);
u32 _XAie_GetFatalGroupErrors(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
u32 _XAie_GetTileBitPosFromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
void _XAie_SetBitInBitmap(u32 *Bitmap, u32 StartSetBit, u32 NumSetBit);
void _XAie_ClrBitInBitmap(u32 *Bitmap, u32 StartSetBit, u32 NumSetBit);
AieRC _XAie_Txn_Start(XAie_DevInst *DevInst, u32 Flags);
AieRC _XAie_Txn_Submit(XAie_DevInst *DevInst, XAie_TxnInst *TxnInst);
XAie_TxnInst* _XAie_TxnExport(XAie_DevInst *DevInst);
u8* _XAie_TxnExportSerialized(XAie_DevInst *DevInst, u8 NumConsumers,
		u32 Flags);
AieRC _XAie_ClearTransaction(XAie_DevInst* DevInst);
AieRC _XAie_TxnFree(XAie_TxnInst *Inst);
void _XAie_TxnResourceCleanup(XAie_DevInst *DevInst);
void _XAie_FreeTxnPtr(void *Ptr);
u8 _XAie_IsUcModulePresent(XAie_DevInst* DevInst, u8 TileType);
void _XAie_AppendPartitionToList(XAie_DevInst *DevInst, XAie_PartitionList *PartInst);
AieRC _XAie_DestroyPartitionFdList(XAie_DevInst *DevInst);
AieRC _XAie_PrintPartitionList(XAie_DevInst *DevInst);
int _XAie_MatchPartitionList(XAie_DevInst *DevInst, u32 PartitionId);
#endif
