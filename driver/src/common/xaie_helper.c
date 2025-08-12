/******************************************************************************
* Copyright (C) 2019-2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_helper.c
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
* 1.1   Tejus   09/24/2019  Fix range check logic for shim row
* 1.2   Tejus   01/04/2020  Cleanup error messages
* 1.3   Tejus   04/13/2020  Add api to get tile type from Loc
* 1.4   Tejus   04/13/2020  Remove helper functions for range apis
* 1.5   Dishita 04/29/2020  Add api to check module & tile type combination
* 1.6   Nishad  07/06/2020  Add _XAie_GetMstrIdx() helper API and move
*			    _XAie_GetSlaveIdx() API.
* 1.7   Nishad  07/24/2020  Add _XAie_GetFatalGroupErrors() helper function.
* 1.8   Dishita 08/10/2020  Add api to get bit position from tile location
* 1.9   Nishad  08/26/2020  Fix tiletype check in _XAie_CheckModule()
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "xaie_helper.h"
#include "xaie_helper_internal.h"
#include "xaie_reset_aie.h"
#include "xaie_txn.h"

/************************** Constant Definitions *****************************/
#define U64_MAX 0xFFFFFFFFFFFFFFFFU

/************************** Variable Definitions *****************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/******************************************************************************/
/**
*
* This is the function used to get the tile type for a given device instance
* and tile location.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	TileType (AIETILE/MEMTILE/SHIMPL/SHIMNOC on success and MAX on
*		error)
*
* @note		Internal API only.
*
******************************************************************************/
u8 XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 ColType;

	if(Loc.Col >= DevInst->NumCols) {
		XAIE_ERROR("Invalid column: %d\n", Loc.Col);
		return XAIEGBL_TILE_TYPE_MAX;
	}

	if(Loc.Row == 0U) {
		ColType = Loc.Col % 4U;
		if((ColType == 0U) || (ColType == 1U)) {
			return XAIEGBL_TILE_TYPE_SHIMPL;
		}

		return XAIEGBL_TILE_TYPE_SHIMNOC;

	} else if(Loc.Row >= DevInst->MemTileRowStart &&
			(Loc.Row < (DevInst->MemTileRowStart +
				     DevInst->MemTileNumRows))) {
		return XAIEGBL_TILE_TYPE_MEMTILE;
	} else if (Loc.Row >= DevInst->AieTileRowStart &&
			(Loc.Row < (DevInst->AieTileRowStart +
				     DevInst->AieTileNumRows))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAIE_ERROR("Cannot find Tile Type\n");

	return XAIEGBL_TILE_TYPE_MAX;
}

/*****************************************************************************/
/**
*
* This is the function used to get the tile type for a given device instance
* and tile location. This function will be removed once other teams migrate
* to the new global function
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	TileType (AIETILE/MEMTILE/SHIMPL/SHIMNOC on success and MAX on
*		error)
*
* @note		Internal API only.
*
******************************************************************************/

u8 _XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	return XAie_GetTileTypefromLoc(DevInst, Loc);
}

/*******************************************************************************/
/**
* This function is used to check for module and tiletype combination.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the AIE tile.
* @param	Module:	XAIE_MEM_MOD - memory module
* 			XAIE_CORE_MOD - core module
* 			XAIE_PL_MOD - pl module
* @return       XAIE_OK for correct combination of Module and tile type
* 		XAIE_INVALID_ARGS for incorrect combination of module and tile
* 		type
*
* @note         Internal API only.
*
*******************************************************************************/

AieRC XAie_CheckModule(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module)
{
	u8 TileType;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_AIETILE && Module > XAIE_CORE_MOD) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if((TileType == XAIEGBL_TILE_TYPE_SHIMPL ||
	    TileType == XAIEGBL_TILE_TYPE_SHIMNOC) && Module != XAIE_PL_MOD) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	if(TileType == XAIEGBL_TILE_TYPE_MEMTILE &&
		Module != XAIE_MEM_MOD) {
		XAIE_ERROR("Invalid Module\n");
		return XAIE_INVALID_ARGS;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This function is used to check for module and tiletype combination.
* This function will be removed once other teams migrate to the new global function
*
* @param        DevInst: Device Instance
* @param        Loc: Location of the AIE tile.
* @param	Module:	XAIE_MEM_MOD - memory module
* 			XAIE_CORE_MOD - core module
* 			XAIE_PL_MOD - pl module
* @return       XAIE_OK for correct combination of Module and tile type
* 		XAIE_INVALID_ARGS for incorrect combination of module and tile
* 		type
*
* @note         Internal API only.
*
*******************************************************************************/
AieRC _XAie_CheckModule(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module)
{
	return XAie_CheckModule(DevInst, Loc, Module);
}

/*******************************************************************************/
/**
* This function is used to get no. of rows for the given tiletype.
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       BitmapNumRows: Number of rows for given tiletype
*
* @note         Internal API only.
*
*******************************************************************************/
u32 XAie_GetNumRows(XAie_DevInst *DevInst, u8 TileType)
{
	u32 NumRows;

	switch(TileType) {
	case XAIEGBL_TILE_TYPE_SHIMNOC:
	case XAIEGBL_TILE_TYPE_SHIMPL:
	{       NumRows = 1U;
		break;
	}
	case XAIEGBL_TILE_TYPE_AIETILE:
	{	NumRows = DevInst->AieTileNumRows;
		break;
	}
	case XAIEGBL_TILE_TYPE_MEMTILE:
	{	NumRows = DevInst->MemTileNumRows;
		break;
	}
	default:
	{
		XAIE_ERROR("Invalid Tiletype\n");
		return 0;
	}
	}

	return NumRows;
}

/*****************************************************************************/
/**
* This function is used to get no. of rows for the given tiletype.
* This function will be removed once other teams migrate to the new global function
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       BitmapNumRows: Number of rows for given tiletype
*
* @note         Internal API only.
*
*******************************************************************************/
u32 _XAie_GetNumRows(XAie_DevInst *DevInst, u8 TileType)
{
	return XAie_GetNumRows(DevInst, TileType);
}


/*******************************************************************************/
/**
* This function is used to get start row for the given tiletype.
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       StartRow: Start row for given tiletype
*
* @note         Internal API only.
*
*******************************************************************************/
u32 XAie_GetStartRow(XAie_DevInst *DevInst, u8 TileType)
{
	u32 StartRow;

	switch(TileType) {
	case XAIEGBL_TILE_TYPE_SHIMNOC:
	case XAIEGBL_TILE_TYPE_SHIMPL:
	{	StartRow = DevInst->ShimRow;
		break;
	}
	case XAIEGBL_TILE_TYPE_AIETILE:
	{
		StartRow = DevInst->AieTileRowStart;
		break;
	}
	case XAIEGBL_TILE_TYPE_MEMTILE:
	{
		StartRow = DevInst->MemTileRowStart;
		break;
	}
	default:
	{
		XAIE_ERROR("Invalid Tiletype\n");
		return 0;
	}
	}

	return StartRow;
}

/*****************************************************************************/
/**
* This function is used to get start row for the given tiletype.
* This function will be removed once other teams migrate to the new global function
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       StartRow: Start row for given tiletype
*
* @note         Internal API only.
*
*******************************************************************************/
u32 _XAie_GetStartRow(XAie_DevInst *DevInst, u8 TileType)
{
	return XAie_GetStartRow(DevInst, TileType);
}

/*****************************************************************************/
/**
*
* To configure stream switch master registers, slave index has to be calculated
* from the internal data structure. The routine calculates the slave index for
* any tile type.
*
* @param	StrmMod: Stream Module pointer
* @param	Slave: Stream switch port type
* @param	PortNum: Slave port number
* @param	SlaveIdx: Place holder for the routine to store the slave idx
*
* @return	XAIE_OK on success and XAIE_INVALID_RANGE on failure
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie_GetSlaveIdx(const XAie_StrmMod *StrmMod, StrmSwPortType Slave,
		u8 PortNum, u8 *SlaveIdx)
{
	u32 BaseAddr;
	u32 RegAddr;
	const XAie_StrmPort *PortPtr;

	/* Get Base Addr of the slave tile from Stream Switch Module */
	BaseAddr = StrmMod->SlvConfigBaseAddr;

	PortPtr = &StrmMod->SlvConfig[Slave];

	/* Return error if the Slave Port Type is not valid */
	if((PortPtr->NumPorts == 0U) || (PortNum >= PortPtr->NumPorts)) {
		XAIE_ERROR("Invalid Slave Port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	RegAddr = PortPtr->PortBaseAddr + StrmMod->PortOffset * PortNum;
	*SlaveIdx = (u8)((RegAddr - BaseAddr) / 4U);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* The routine calculates the master index for any tile type.
*
* @param	StrmMod: Stream Module pointer
* @param	Master: Stream switch port type
* @param	PortNum: Master port number
* @param	MasterIdx: Place holder for the routine to store the master idx
*
* @return	XAIE_OK on success and XAIE_INVALID_RANGE on failure
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie_GetMstrIdx(const XAie_StrmMod *StrmMod, StrmSwPortType Master,
		u8 PortNum, u8 *MasterIdx)
{
	u32 BaseAddr;
	u32 RegAddr;
	const XAie_StrmPort *PortPtr;

	/* Get Base Addr of the master tile from Stream Switch Module */
	BaseAddr = StrmMod->MstrConfigBaseAddr;

	PortPtr = &StrmMod->MstrConfig[Master];

	/* Return error if the Master Port Type is not valid */
	if((PortPtr->NumPorts == 0U) || (PortNum >= PortPtr->NumPorts)) {
		XAIE_ERROR("Invalid Master Port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	RegAddr = PortPtr->PortBaseAddr + StrmMod->PortOffset * PortNum;
	*MasterIdx = (u8)((RegAddr - BaseAddr) / 4U);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the default value of group errors marked as fatal.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
*
* @return	Default value of group fatal errors.
*
* @note		Internal API only.
*
******************************************************************************/
u32 _XAie_GetFatalGroupErrors(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module)
{
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if (Module == XAIE_PL_MOD) {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	return EvntMod->DefaultGroupErrorMask;
}

void XAie_Log(FILE *Fd, const char *prefix, const char *func, u32 line,
		const char *Format, ...)
{
	va_list ArgPtr;
	va_start(ArgPtr, Format);
	if (fprintf(Fd, "%s %s():%u: ", prefix, func, line) == -1) {
		va_end(ArgPtr);
		return;
	}
	if (vfprintf(Fd, Format, ArgPtr) == -1){
		va_end(ArgPtr);
		return;
	}
	va_end(ArgPtr);
}

/*****************************************************************************/
/**
* This is an internal API to get bit position corresponding to tile location in
* bitmap. This bitmap does not represent Shim tile so this API
* only accepts AIE tile.
*
* @param        DevInst: Device Instance
* @param        Loc: Location of AIE tile
* @return       Bit position in the TilesInUse bitmap
*
* @note         None
*
******************************************************************************/
u32 _XAie_GetTileBitPosFromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	return (u32)(Loc.Col * (DevInst->NumRows - 1U) + Loc.Row - 1U);
}

/*******************************************************************************/
/**
 * This API populates ungated tiles of partition to Locs list.
 *
 * @param        DevInst: Device Instance
 * @param        NumTiles: Size of Locs array.
 * @param        Locs: Pointer to tile locations list
 *
 * @note         NumTiles pointer is used to indicate the size of Locs as input
 *               when passed by the caller. The same pointer gets updated to
 *               indicate the return locs list size.
 *
 *******************************************************************************/
AieRC XAie_GetUngatedLocsInPartition(XAie_DevInst *DevInst, u32 *NumTiles,
		XAie_LocType *Locs)
{
	u32 Index = 0;

	/* Add clock enabled tiles of the partition to Rscs */
	for(u8 Col = 0; Col < DevInst->NumCols; Col++) {
		for(u8 Row = 0; Row < DevInst->NumRows; Row++) {
			XAie_LocType Loc = XAie_TileLoc(Col, Row);

			if(_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_ENABLE) {
				if(Index >= *NumTiles) {
					XAIE_ERROR("Invalid NumTiles: %d\n",
							*NumTiles);
					return XAIE_INVALID_ARGS;
				}

				Locs[Index] = Loc;
				Index++;
			}
		}
	}

	/* Update NumTiles to size equal to ungated tiles in partition */
	*NumTiles = Index;
	return XAIE_OK;
}

/*****************************************************************************/
/**
 * This API populates ungated tiles of partition to Locs list.
* This function will be removed once other teams migrate to the new global function
 *
 * @param        DevInst: Device Instance
 * @param        NumTiles: Size of Locs array.
 * @param        Locs: Pointer to tile locations list
 *
 * @note         NumTiles pointer is used to indicate the size of Locs as input
 *               when passed by the caller. The same pointer gets updated to
 *               indicate the return locs list size.
 *
 *******************************************************************************/
AieRC _XAie_GetUngatedLocsInPartition(XAie_DevInst *DevInst, u32 *NumTiles,
		XAie_LocType *Locs)
{
	return XAie_GetUngatedLocsInPartition(DevInst, NumTiles, Locs);
}

/*****************************************************************************/
/**
* This API sets given number of bits from given start bit in the given bitmap.
*
* @param        Bitmap: bitmap to be set
* @param        StartSetBit: Bit position in the bitmap
* @param        NumSetBit: Number of bits to be set.
*
* @return       none
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*
******************************************************************************/
void _XAie_SetBitInBitmap(u32 *Bitmap, u32 StartSetBit,
		u32 NumSetBit)
{
	for(u32 i = StartSetBit; i < StartSetBit + NumSetBit; i++) {
		Bitmap[i / (sizeof(Bitmap[0]) * 8U)] |=
			(u32)(1U << (i % (sizeof(Bitmap[0]) * 8U)));
	}
}

/*****************************************************************************/
/**
* This API clears number of bits from given start bit in the given bitmap.
*
* @param        Bitmap: bitmap to be set
* @param        StartBit: Bit position in the bitmap
* @param        NumBit: Number of bits to be set.
*
* @return       None
*
* @note         This API is internal, hence all the argument checks are taken
*               care of in the caller API.
*
******************************************************************************/
void _XAie_ClrBitInBitmap(u32 *Bitmap, u32 StartBit, u32 NumBit)
{
	for(u32 i = StartBit; i < StartBit + NumBit; i++) {
		Bitmap[i / (sizeof(Bitmap[0]) * 8U)] &=
			~(u32)((1U << (i % (sizeof(Bitmap[0]) * 8U))));
	}
}

AieRC XAie_Write32(XAie_DevInst *DevInst, u64 RegOff, u32 Value)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_Write32(DevInst, RegOff, Value);
	}

	return Backend->Ops.Write32((void*)(DevInst->IOInst), RegOff, Value);
}

AieRC XAie_Read32(XAie_DevInst *DevInst, u64 RegOff, u32 *Data)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_Read32(DevInst, RegOff, Data);
	}

	return Backend->Ops.Read32((void*)(DevInst->IOInst), RegOff, Data);
}

AieRC XAie_MaskWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_MaskWrite32(DevInst, RegOff, Mask, Value);
	}

	return Backend->Ops.MaskWrite32((void *)(DevInst->IOInst), RegOff, Mask,
			Value);
}

AieRC XAie_MaskPoll(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_MaskPoll(DevInst, RegOff, Mask, Value, TimeOutUs);
	}

	return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
			Value, TimeOutUs);
}

AieRC XAie_MaskPollBusy(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_MaskPollBusy(DevInst, RegOff, Mask, Value, TimeOutUs);
	}
	return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
			Value, TimeOutUs);
}

AieRC XAie_BlockWrite32(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data, u32 Size)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_BlockWrite32(DevInst, RegOff, Data, Size);
	}

	return Backend->Ops.BlockWrite32((void *)(DevInst->IOInst), RegOff,
			Data, Size);
}

AieRC XAie_BlockSet32(XAie_DevInst *DevInst, u64 RegOff, u32 Data, u32 Size)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		return XAie_Txn_BlockSet32(DevInst, RegOff, Data, Size);
	}

	return Backend->Ops.BlockSet32((void *)(DevInst->IOInst), RegOff, Data,
			Size);
}

AieRC XAie_CmdWrite(XAie_DevInst *DevInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		/**
		 * This function is only used in XAie_LoadElf() on AIESIM platform.
		 * In the past the elf loading was done via XCLBIN (PDI) but not
		 * via TXN binary. Hence this unwanted TXN implementation did not have
		 * any side effects. But when elf loading is attempted via TXN flow 
		 * this needs to be made a NOOP else it fails. Hence Making 
		 * XAIe_CmdWrite no-op for transaction mode.
		 **/
		return XAIE_OK;
	}
	return Backend->Ops.CmdWrite((void *)(DevInst->IOInst), Col, Row,
			Command, CmdWd0, CmdWd1, CmdStr);
}

AieRC XAie_RunOp(XAie_DevInst *DevInst, XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		RC = XAie_Txn_RunOp(DevInst, (u8)Op == XAIE_BACKEND_OP_CONFIG_SHIMDMABD, Arg);
		if (RC == XAIE_NOT_SUPPORTED)
			return Backend->Ops.RunOp(DevInst->IOInst, DevInst, Op, Arg);
		else
			return RC;
	}

	return Backend->Ops.RunOp(DevInst->IOInst, DevInst, Op, Arg);
}

AieRC XAie_AddressPatching(XAie_DevInst *DevInst, u8 Arg_Offset, u8 Num_BDs)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (Backend->Ops.AddressPatching != NULL) {
		return Backend->Ops.AddressPatching((void *)DevInst->IOInst, Arg_Offset, Num_BDs);
	} else {
		return XAIE_FEATURE_NOT_SUPPORTED;
	}
}

/*****************************************************************************/
/**
*
* This API returns the Aie Tile Core status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Loc: Location of AIE tile
*
* @return	XAIE_OK for success and error code otherwise.
*
* @note	Internal only.
*
******************************************************************************/
static AieRC _XAie_CoreStatusDump(XAie_DevInst *DevInst,
		XAie_ColStatus *Status, XAie_LocType Loc)
{
	u32 RegVal;
	AieRC RC;
	u8 TileType, TileStart, Index;

	TileStart = DevInst->AieTileRowStart;
	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	/* core status */
	RC = XAie_CoreGetStatus(DevInst, Loc, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}
	if(Loc.Row < TileStart){
		XAIE_ERROR("Loc.Row should not be less than TileStart\n");
		return XAIE_ERR;
	}else {
		Index = Loc.Row - TileStart;
	}
	Status[Loc.Col].CoreTile[Index].CoreStatus = RegVal;

	/* program counter */
	RC = XAie_CoreGetPCValue(DevInst, Loc, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}
	Status[Loc.Col].CoreTile[Index].ProgramCounter = RegVal;

	/* stack pointer */
	RC = XAie_CoreGetSPValue(DevInst, Loc, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}
	Status[Loc.Col].CoreTile[Index].StackPtr = RegVal;

	/* link register */
	RC = XAie_CoreGetLRValue(DevInst, Loc, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}
	Status[Loc.Col].CoreTile[Index].LinkReg = RegVal;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the All Tile DMA Channel status for a particular column
* and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Loc: Location of AIE tile
*
* @return	XAIE_OK for success and error code otherwise.
*
* @note	Internal only.
*
******************************************************************************/
static AieRC _XAie_DmaStatusDump(XAie_DevInst *DevInst,
		XAie_ColStatus *Status, XAie_LocType Loc)
{
	u32 RegVal;
	u8 TileType, AieTileStart, MemTileStart;
	u8 Index;
	AieRC RC;

	const XAie_DmaMod *DmaMod;
	XAie_CoreTileStatus *CoreTile;
	XAie_ShimTileStatus *ShimTile;
	XAie_MemTileStatus *MemTile;

	AieTileStart = DevInst->AieTileRowStart;
	MemTileStart = DevInst->MemTileRowStart;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if((TileType >= XAIEGBL_TILE_TYPE_MAX) ||
			(TileType == XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	CoreTile = Status[Loc.Col].CoreTile;
	MemTile = Status[Loc.Col].MemTile;
	ShimTile = Status[Loc.Col].ShimTile;

	/* iterate all tile dma channels */
	for (u8 Chan = 0; Chan < DmaMod->NumChannels; Chan++) {

		/* read s2mm channel status */
		RC = XAie_DmaGetChannelStatus(DevInst, Loc, Chan,
				DMA_S2MM, &RegVal);
		if (RC != XAIE_OK) {
			return RC;
		}

		if(Loc.Row < AieTileStart ||  Loc.Row < MemTileStart){
			XAIE_ERROR("Loc.Row should not be less than TileStart\n");
			return XAIE_ERR;
		}

		if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
			Index = Loc.Row - AieTileStart;
			CoreTile[Index].Dma[Chan].S2MMStatus = RegVal;
		} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
			Index = Loc.Row - MemTileStart;
			MemTile[Index].Dma[Chan].S2MMStatus = RegVal;
		} else {
			ShimTile[Loc.Row].Dma[Chan].S2MMStatus = RegVal;
		}

		/* read mm2s channel status */
		RC = XAie_DmaGetChannelStatus(DevInst, Loc, Chan,
				DMA_MM2S, &RegVal);
		if (RC != XAIE_OK) {
			return RC;
		}

		if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
			Index = Loc.Row - AieTileStart;
			CoreTile[Index].Dma[Chan].MM2SStatus = RegVal;
		} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
			Index = Loc.Row - MemTileStart;
			MemTile[Index].Dma[Chan].MM2SStatus = RegVal;
		} else {
			ShimTile[Loc.Row].Dma[Chan].MM2SStatus = RegVal;
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the Aie Lock status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Loc: Location of AIE tile
*
* @return	XAIE_OK for success and error code otherwise.
*
* @note	Internal only.
*
******************************************************************************/
static AieRC _XAie_LockValueStatusDump(XAie_DevInst *DevInst,
		XAie_ColStatus *Status, XAie_LocType Loc)
{
	u32 RegVal;
	u8 TileType, AieTileStart, MemTileStart;
	u8 Index;
	AieRC RC;
	XAie_Lock Lock = {0,0};

	const XAie_LockMod *LockMod;
	XAie_CoreTileStatus *CoreTile;
	XAie_ShimTileStatus *ShimTile;
	XAie_MemTileStatus *MemTile;

	AieTileStart = DevInst->AieTileRowStart;
	MemTileStart = DevInst->MemTileRowStart;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if((TileType >= XAIEGBL_TILE_TYPE_MAX) ||
			(TileType == XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(Loc.Row < AieTileStart ||  Loc.Row < MemTileStart){
		XAIE_ERROR("Loc.Row should not be less than TileStart\n");
		return XAIE_ERR;
	}

	LockMod = DevInst->DevProp.DevMod[TileType].LockMod;
	CoreTile = Status[Loc.Col].CoreTile;
	MemTile = Status[Loc.Col].MemTile;
	ShimTile = Status[Loc.Col].ShimTile;

	/* iterate all lock value registers */
	for(u32 LockCnt = 0; LockCnt < LockMod->NumLocks; LockCnt++) {

		/* read lock value */
		Lock.LockId = (u8)LockCnt;
		RC = XAie_LockGetValue(DevInst, Loc, Lock, &RegVal);
		if (RC != XAIE_OK) {
			return RC;
		}

		if(RegVal > UINT8_MAX){
			XAIE_ERROR("Invalid Lock value\n");
			return XAIE_ERR;
		}

		if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
			Index = Loc.Row - AieTileStart;
			CoreTile[Index].LockValue[LockCnt] = (u8)RegVal;
		} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
			Index = Loc.Row - MemTileStart;
			MemTile[Index].LockValue[LockCnt] = (u8)RegVal;
		} else {
			ShimTile[Loc.Row].LockValue[LockCnt] = (u8)RegVal;
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the Tile Event Status for a particular column and row.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
* @param	Loc: Location of AIE tile
*
* @return	XAIE_OK for success and error code otherwise.
*
* @note	Internal only.
*
******************************************************************************/
static AieRC _XAie_EventStatusDump(XAie_DevInst *DevInst,
		XAie_ColStatus *Status, XAie_LocType Loc)
{
	u32 RegVal;
	AieRC RC;
	u8 TileType, AieTileStart, MemTileStart;
	u8 Index;
	u8 NumEventReg;

	const XAie_EvntMod *EvntCoreMod, *EvntMod;
	XAie_CoreTileStatus *CoreTile;
	XAie_ShimTileStatus *ShimTile;
	XAie_MemTileStatus *MemTile;
	const XAie_TileMod *DevMod;

	AieTileStart = DevInst->AieTileRowStart;
	MemTileStart = DevInst->MemTileRowStart;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType >= XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	CoreTile = Status[Loc.Col].CoreTile;
	MemTile = Status[Loc.Col].MemTile;
	ShimTile = Status[Loc.Col].ShimTile;
	DevMod = DevInst->DevProp.DevMod;

	if(Loc.Row < AieTileStart || Loc.Row < MemTileStart ){
		XAIE_ERROR("Loc.Row should not be less than TileStart\n");
		return XAIE_ERR;
	}

	if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
		EvntCoreMod = &DevMod[TileType].EvntMod[XAIE_CORE_MOD];
		Index = Loc.Row - AieTileStart;
		/* iterate all event status registers */
		NumEventReg = EvntCoreMod->NumEventReg;
		for(u32 EventReg = 0; EventReg < NumEventReg; EventReg++) {
			/* read event status register and store in output
			 * buffer */
			RC = XAie_EventRegStatus(DevInst, Loc, XAIE_CORE_MOD,
					(u8)EventReg, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}

			CoreTile[Index].EventCoreModStatus[EventReg] = RegVal;

			/* read event status register and store in output
			 * buffer */
			RC = XAie_EventRegStatus(DevInst, Loc, XAIE_MEM_MOD,
					(u8)EventReg, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}
			CoreTile[Index].EventMemModStatus[EventReg] = RegVal;
		}
	} else if(TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
		EvntMod = &DevMod[TileType].EvntMod[XAIE_MEM_MOD];
		Index = Loc.Row - MemTileStart;
		NumEventReg = EvntMod->NumEventReg;
		for(u32 EventReg = 0; EventReg < NumEventReg; EventReg++) {
			RC = XAie_EventRegStatus(DevInst, Loc, XAIE_MEM_MOD,
					(u8)EventReg, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}
			MemTile[Index].EventStatus[EventReg] = RegVal;
		}
	} else {
		EvntMod = &DevMod[TileType].EvntMod[0U];
		NumEventReg = EvntMod->NumEventReg;
		for(u32 EventReg = 0; EventReg < NumEventReg; EventReg++) {
			RC = XAie_EventRegStatus(DevInst, Loc, XAIE_PL_MOD,
					(u8)EventReg, &RegVal);
			if (RC != XAIE_OK) {
				return RC;
			}
			ShimTile[Loc.Row].EventStatus[EventReg] = RegVal;
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the column status for N number of colums.
*
* @param	DevInst: Device Instance
* @param	Status: Pointer to user defined column status buffer.
*
* @return	XAIE_OK for success and error code otherwise.
*
* @note	None.
*
******************************************************************************/
AieRC XAie_StatusDump(XAie_DevInst *DevInst, XAie_ColStatus *Status)
{
	AieRC RC = XAIE_ERR;
	u8 StartCol = DevInst->StartCol;
	u8 NumCols  = DevInst->NumCols;
	u8 NumRows  = DevInst->NumRows;
	XAie_LocType Loc;

	if(Status == NULL) {
		return XAIE_ERR;
	}

	/* iterate specified columns */
	for(u8 Col = StartCol; Col < NumCols; Col++) {
		for(u8 Row = 0; Row < NumRows; Row++) {
			Loc.Row = Row;
			Loc.Col = Col;
			RC |= (u32)_XAie_CoreStatusDump(DevInst,Status, Loc);
			RC |= (u32)_XAie_DmaStatusDump(DevInst, Status, Loc);
			RC |= (u32)_XAie_LockValueStatusDump(DevInst, Status, Loc);
			RC |= (u32)_XAie_EventStatusDump(DevInst, Status, Loc);
		}
	}
	return (AieRC)RC;
}

/*****************************************************************************/
/**
*
* This routine is used to check if given lsb & value pair exceeds precision.
*
* @param	Lsb				: Shift Value
* @param    ValueBitCount	: Value to be shifted
* @param    MaxValidBitPos	: Max desired precision post shift
*
* @return	0 - if precision intact else precision exceeds
*
* @note		Internal API only.
*
*******************************************************************************/
u8 _XAie_CheckPrecisionExceeds(u32 Lsb, u8 ValueBitCount, u8 MaxValidBitPos)
{
	if ((Lsb + ValueBitCount) > MaxValidBitPos) {
		return 1;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This routine is used calculate the max bits needed for a given integer number.
*
* @param    Value		: Value
*
* @return	No of bits needed to represent the value.
*
* @note		Internal API only.
*
*******************************************************************************/
u8 _XAie_MaxBitsNeeded(u32 value)
{
    // Calculate the number of bits needed to represent the value
    if (value == 0) {
        return 1; // Special case for zero
    }

    u8 bits = 0;
    while (value) {
        bits++;
        value >>= 1; // Right shift by 1 (equivalent to dividing by 2)
    }
    return bits;
}

/*****************************************************************************/
/**
*
* This routine is used to check if given lsb & Mask pair exceeds precision
* when a masked value is right shift by lsb
*
* @param	Lsb				: Shift Value
* @param    Mask			: Value to be shifted
*
* @return	0 - if precision intact else precision exceeds
*
* @note		Internal API only.
*
*******************************************************************************/
u8 _XAie_CheckPrecisionExceedsForRightShift(u32 Lsb, u32 Mask)
{
	if (Lsb  > _XAie_CountTrailingZeros(Mask)) {
		return 1;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This routine is used calculate the trailing zeros needed for a given 
* integer number in  binary form
*
* @param    Value		: Value
*
* @return	No of trailing zeros in the given number.
*
* @note		Internal API only.
*
*******************************************************************************/
u8 _XAie_CountTrailingZeros(u32 value)
{
	u8 count = 0;
    while ((value & 1) == 0 && value > 0)  {
		value >>= 1;
        count++; // Right shift by 1 (equivalent to dividing by 2)
    }
    return count;
}


/** @} */
