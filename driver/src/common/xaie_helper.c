/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
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
#include "xaie_reset_aie.h"
#include "xaie_txn.h"

/************************** Constant Definitions *****************************/
#define XAIE_DEFAULT_NUM_CMDS 1024U
#define XAIE_DEFAULT_TXN_BUFFER_SIZE (1024 * 4)

#define XAIE_TXN_INSTANCE_EXPORTED	0b10U
#define XAIE_TXN_INST_EXPORTED_MASK XAIE_TXN_INSTANCE_EXPORTED
#define XAIE_TXN_AUTO_FLUSH_MASK XAIE_TRANSACTION_ENABLE_AUTO_FLUSH

#define TX_DUMP_ENABLE 0
#define XAIE_INVALID_PARTITIONFD -1

#define XAIE_TILEDMA_NUM_BD_WORDS			7U
#define XAIE_SHIMDMA_NUM_BD_WORDS			5U

#define XAIEML_TILEDMA_NUM_BD_WORDS			6U
#define XAIEML_SHIMDMA_NUM_BD_WORDS			8U
#define XAIEML_MEMTILEDMA_NUM_BD_WORDS			8U

#define XAIE2PS_TILEDMA_NUM_BD_WORDS			6U
#define XAIE2PS_SHIMDMA_NUM_BD_WORDS			9U
#define XAIE2PS_MEMTILEDMA_NUM_BD_WORDS		8U
/************************** Variable Definitions *****************************/
const u8 TransactionHeaderVersion_Major = 0;
const u8 TransactionHeaderVersion_Minor = 1;

/**
 Define the global log level variable with a default value.
 If the environment variable isn't set, this level will be used.
 ***************************************************************/
XAieLogLevel AieLogLevel = XAIE_LOG_LEVEL_ERROR;
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
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

	XAIE_ERROR("Cannot find Tile Type. Col: %u Row:%u \n", Loc.Col, Loc.Row);

	return XAIEGBL_TILE_TYPE_MAX;
}

/*****************************************************************************/
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
*******************************************************************************/
AieRC XAie_CheckModule(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module)
{
	u8 TileType;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_AIETILE && Module > XAIE_CORE_MOD) {
		XAIE_ERROR("Invalid Module.Col:%u Row:%u TileType:%u Module:%u\n",Loc.Col, Loc.Row, TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	if((TileType == XAIEGBL_TILE_TYPE_SHIMPL ||
	    TileType == XAIEGBL_TILE_TYPE_SHIMNOC) && Module != XAIE_PL_MOD) {
		XAIE_ERROR("Invalid Module.Col:%u Row:%u TileType:%u Module:%u\n",Loc.Col, Loc.Row, TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	if(TileType == XAIEGBL_TILE_TYPE_MEMTILE &&
		Module != XAIE_MEM_MOD) {
		XAIE_ERROR("Invalid Module.Col:%u Row:%u TileType:%u Module:%u\n",Loc.Col, Loc.Row, TileType, Module);
		return XAIE_INVALID_ARGS;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This function is used to get no. of rows for the given tiletype.
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       BitmapNumRows: Number of rows for given tiletype
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
		XAIE_ERROR("Invalid Tiletype. TileType: %u\n", TileType);
		return 0;
	}
	}

	return NumRows;
}

/*****************************************************************************/
/**
* This function is used to get start row for the given tiletype.
*
* @param        DevInst: Device Instance
* @param        TileType: Type of tile
*
* @return       StartRow: Start row for given tiletype
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
		XAIE_ERROR("Invalid Tiletype. TileType: %u\n", TileType);
		return 0;
	}
	}

	return StartRow;
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
		XAIE_ERROR("Invalid Master Port. NumPorts: %u PortNum: %u\n",PortPtr->NumPorts, PortNum);
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
	if(Module == XAIE_PL_MOD) {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	return EvntMod->DefaultGroupErrorMask;
}

/**
 * Case-insensitive string comparison implementation for embedded systems
 * that may not have strcasecmp() available.
 */
static int xaie_strcasecmp(const char *s1, const char *s2) {
	unsigned char c1, c2;

	while (*s1 && *s2) {
		c1 = (unsigned char)*s1;
		c2 = (unsigned char)*s2;

		/* Convert to lowercase */
		if (c1 >= 'A' && c1 <= 'Z')
			c1 += 'a' - 'A';
		if (c2 >= 'A' && c2 <= 'Z')
			c2 += 'a' - 'A';

		if (c1 != c2)
			return c1 - c2;

		s1++;
		s2++;
	}

	/* Handle different string lengths */
	c1 = (unsigned char)*s1;
	c2 = (unsigned char)*s2;
	if (c1 >= 'A' && c1 <= 'Z')
		c1 += 'a' - 'A';
	if (c2 >= 'A' && c2 <= 'Z')
		c2 += 'a' - 'A';

	return c1 - c2;
}

/**
 * This function is automatically executed when the shared library is loaded.
 * It reads the "XAIE_LOG_LEVEL" environment variable to configure the logger.
 * Note: __attribute__((constructor)) is a GCC/Clang language extension.
 * If the environment variable is not set or has an invalid value,
 * the default AieLogLevel (XAIE_LOG_LEVEL_ERROR) remains active.
 */
__attribute__((constructor))
static void _XAie_LoggerInit(void) {
    const char *AieLevelEnv = getenv("XAIE_LOG_LEVEL");
	XAie_LoggerInit(AieLevelEnv);
}

void XAie_LoggerInit(const char *AieLevelEnv) {
    if (AieLevelEnv) {
        if (xaie_strcasecmp(AieLevelEnv, "FATAL") == 0) AieLogLevel = XAIE_LOG_LEVEL_FATAL;
        else if (xaie_strcasecmp(AieLevelEnv, "ERROR") == 0) AieLogLevel = XAIE_LOG_LEVEL_ERROR;
        else if (xaie_strcasecmp(AieLevelEnv, "WARN")  == 0) AieLogLevel = XAIE_LOG_LEVEL_WARN;
        else if (xaie_strcasecmp(AieLevelEnv, "INFO")  == 0) AieLogLevel = XAIE_LOG_LEVEL_INFO;
        else if (xaie_strcasecmp(AieLevelEnv, "DEBUG") == 0) AieLogLevel = XAIE_LOG_LEVEL_DEBUG;
        else if (xaie_strcasecmp(AieLevelEnv, "TRACE") == 0) AieLogLevel = XAIE_LOG_LEVEL_TRACE;
    }
}

void XAie_Log(FILE *Fd, const char *prefix, const char *func, u32 line,
		const char *Format, ...)
{
	va_list ArgPtr;
	va_start(ArgPtr, Format);
	fprintf(Fd, "%s %s():"U32_FORMAT": ", prefix, func, line);
	vfprintf(Fd, Format, ArgPtr);
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
	return (u32)((Loc.Col + DevInst->StartCol) * (DevInst->NumRows - 1U) + Loc.Row - 1U);
}

/*****************************************************************************/
/**
* This API populates ungated tiles of partition to Locs list.
*
* @param	DevInst: Device Instance
* @param	NumTiles: Size of Locs array.
* @param	Locs: Pointer to tile locations list
*
* @note		NumTiles pointer is used to indicate the size of Locs as input
* 		when passed by the caller. The same pointer gets updated to
* 		indicate the return locs list size.
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
					XAIE_ERROR("Invalid NumTiles: %d Col: %u Row: %u\n",
						*NumTiles, Loc.Col, Loc.Row);
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

/*****************************************************************************/
/**
* This API will appends the partition information to the partition list.
*
* @param	DevInst: Global AIE Device instance pointer.
*
* @param	PartInst: pointer having the partition information, which
*               aquires from kernel
*
* @return	none
*
******************************************************************************/
void _XAie_AppendPartitionToList(XAie_DevInst *DevInst,
		XAie_PartitionList *PartInst)
{
	XAie_List *Node = &DevInst->PartitionList;

	while(Node->Next != NULL) {
		Node = Node->Next;
	}

	Node->Next = &PartInst->Node;
	PartInst->Node.Next = NULL;

}

/*****************************************************************************/
/**
*
* This function will match the partition from list  and update StartCol & NumCols.
*
* @param	DevInst: Global AIE Device instance pointer.
*
* @param	PartitionId: PartitionId given by user.
*
* @return	 Partition Fd  on success, negetive values on failure.
*
*******************************************************************************/
int _XAie_MatchPartitionList(XAie_DevInst *DevInst, u32 PartitionId)
{
	XAie_List *NodePtr;
	XAie_PartitionList *ListNode;

	if (DevInst->PartitionList.Next == NULL) {
		XAIE_ERROR("Partition list is empty \n");
		return XAIE_INVALID_PARTITIONFD;
	}

	NodePtr = (XAie_List *)&DevInst->PartitionList.Next->Next;

	while (NodePtr != NULL) {
		ListNode = XAIE_CONTAINER_OF(NodePtr, XAie_PartitionList, Node);

		if (PartitionId == ListNode->PartitionId) {
			DevInst->StartCol = ListNode->ColRange.Start;
			DevInst->NumCols = ListNode->ColRange.Num;
			return ListNode->PartitionFd;
		}

		NodePtr = NodePtr->Next;
	}

	XAIE_ERROR("Failed to match the partition id from the list PartitionId: %u  "\
			"  ListNode->PartitionId: %u\n", PartitionId,  ListNode->PartitionId);

	return XAIE_INVALID_PARTITIONFD;
}

/*****************************************************************************/
/**
*
* This is the function to Destory the partitions entry in the list and make it
* free.
*
* @param	DevInst: Global AIE Device instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
AieRC _XAie_DestroyPartitionFdList(XAie_DevInst *DevInst)
{
	XAie_List *NodePtr;
	XAie_PartitionList *ListNode, *DeleteNode;

	if (DevInst->PartitionList.Next == NULL) {
		XAIE_ERROR("Partition list is empty\n");
		return XAIE_ERR;
	}

	NodePtr = (XAie_List *)&DevInst->PartitionList.Next->Next;

	while (NodePtr != NULL) {
		ListNode = XAIE_CONTAINER_OF(NodePtr, XAie_PartitionList, Node);

		DeleteNode = ListNode;
		NodePtr = NodePtr->Next;

		free(DeleteNode);
	}

	DevInst->PartitionList.Next = NULL;
        return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function will print the partitions and it's information from the list.
*
* @param	DevInst: Global AIE Device instance pointer.
*
* @return	XAIE_OK on success, error code on failure.
*
*******************************************************************************/
AieRC _XAie_PrintPartitionList(XAie_DevInst *DevInst)
{
        XAie_List *NodePtr;
        XAie_PartitionList *ListNode;

	if (DevInst->PartitionList.Next == NULL) {
		XAIE_ERROR("Partition list is empty \n");
		return XAIE_ERR;
	}

	NodePtr = (XAie_List *)&DevInst->PartitionList.Next->Next;

	while (NodePtr != NULL) {
		ListNode = XAIE_CONTAINER_OF(NodePtr, XAie_PartitionList, Node);

		PRINT("Partition ID:   "U32_FORMAT"\n",ListNode->PartitionId);
		printf("\tStart Col:    %d \n", ListNode->ColRange.Start);
		printf("\tNum Cols:     %d\n", ListNode->ColRange.Num);
		printf("\tPartition Fd: %d\n", ListNode->PartitionFd);

		NodePtr = NodePtr->Next;
        }

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
		TxnInst = XAIE_CONTAINER_OF(NodePtr, XAie_TxnInst, Node);
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
		Inst = (XAie_TxnInst *)XAIE_CONTAINER_OF(NodePtr, XAie_TxnInst,
				Node);
		if(Inst->Tid == Tid) {
			break;
		}

		Prev = NodePtr;
		NodePtr = NodePtr->Next;
	}

	if(NodePtr == NULL) {
		XAIE_ERROR("Cannot find node to delete from list,  NodePtr is NULL\n");
		return XAIE_ERR;
	} else {
		Prev->Next = NodePtr->Next;
	}

	return XAIE_OK;
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
	BuffHexDump((char *)(uintptr_t)(cmd->DataPtr),cmd->Size);
	return 0;
}

/*****************************************************************************/
/**
* This API rellaocates the command buffer associated with the given transaction
* instance. The command buffer is extended to accomodate an additional
* XAIE_DEFAULT_NUM_CMDS number of commands.
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
	TxnInst->CmdBuf = (XAie_TxnCmd *)realloc((void *)TxnInst->CmdBuf,
			sizeof(XAie_TxnCmd) *
			(u64)(TxnInst->MaxCmds + XAIE_DEFAULT_NUM_CMDS));
	if(TxnInst->CmdBuf == NULL) {
		XAIE_ERROR("Failed reallocate memory for transaction buffer "
				"with id: %d\n", TxnInst->Tid);
		return XAIE_ERR;
	}

	TxnInst->MaxCmds += XAIE_DEFAULT_NUM_CMDS;

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

	Inst->CmdBuf = (XAie_TxnCmd*)calloc(XAIE_DEFAULT_NUM_CMDS,
			sizeof(*Inst->CmdBuf));
	if(Inst->CmdBuf == NULL) {
		XAIE_ERROR("Failed to allocate memory for command buffer\n");
		free(Inst);
		return XAIE_ERR;
	}

	Inst->NumCmds = 0U;
	Inst->MaxCmds = XAIE_DEFAULT_NUM_CMDS;
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
				XAIE_ERROR("Wr failed. Addr: 0x%lx, Mask: 0x%x,"
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
				XAIE_ERROR("Block Wr failed. Addr: 0x%lx\n",
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
				XAIE_ERROR("Block Wr failed. Addr: 0x%lx\n",
						Cmd->RegOff);
				return RC;
			}
			break;
		case XAIE_IO_MASKPOLL:
			/* Force timeout to default value */
			RC = Backend->Ops.MaskPoll((void*)DevInst->IOInst,
							Cmd->RegOff, Cmd->Mask,
							Cmd->Value, 0U);
			if(RC != XAIE_OK) {
				XAIE_ERROR("MP failed. Addr: 0x%lx, Mask: 0x%x, Value: 0x%x\n",
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
				free(Inst->CmdBuf);
				free(Inst);
				return NULL;
			}

			Cmd->DataPtr = (u64)(uintptr_t)memcpy(
					(void *)(uintptr_t)Cmd->DataPtr,
					(void *)(uintptr_t)TmpCmd->DataPtr,
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

static inline void _XAie_CreateTxnHeader(XAie_DevInst *DevInst,
		XAie_TxnHeader *Header)
{
	Header->Major = TransactionHeaderVersion_Major;
	Header->Minor = TransactionHeaderVersion_Minor;
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

static inline void _XAie_AppendWrite32(XAie_DevInst *DevInst,
		XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	XAie_Write32Hdr *Hdr = (XAie_Write32Hdr*)TxnPtr;
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
	XAie_MaskWrite32Hdr *Hdr = (XAie_MaskWrite32Hdr*)TxnPtr;

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
	XAie_MaskPoll32Hdr *Hdr = (XAie_MaskPoll32Hdr*)TxnPtr;

	Hdr->RegOff = Cmd->RegOff;
	Hdr->Mask = Cmd->Mask;
	Hdr->Value = Cmd->Value;
	Hdr->Size = (u32)sizeof(*Hdr);
	Hdr->OpHdr.Col = _XAie_GetColfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Row = _XAie_GetRowfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Op = (u8)XAIE_IO_MASKPOLL;
}

static inline void _XAie_AppendBlockWrite32(XAie_DevInst *DevInst,
		XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	u8 *Payload = TxnPtr + sizeof(XAie_BlockWrite32Hdr);
	XAie_BlockWrite32Hdr *Hdr = (XAie_BlockWrite32Hdr*)TxnPtr;

	Hdr->RegOff = (u32)Cmd->RegOff;
	Hdr->Size = (u32)sizeof(*Hdr) + Cmd->Size * (u32)sizeof(u32);
	Hdr->OpHdr.Col = _XAie_GetColfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Row = _XAie_GetRowfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Op = (u8)XAIE_IO_BLOCKWRITE;

	memcpy((void *)Payload, (void *)(uintptr_t)Cmd->DataPtr,
			Cmd->Size * sizeof(u32));
}

static inline void _XAie_AppendBlockSet32(XAie_DevInst *DevInst,
		XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	u8 *Payload = TxnPtr + sizeof(XAie_BlockWrite32Hdr);
	XAie_BlockWrite32Hdr *Hdr = (XAie_BlockWrite32Hdr*)TxnPtr;

	Hdr->RegOff = (u32)Cmd->RegOff;
	Hdr->Size = (u32)sizeof(*Hdr) + Cmd->Size * (u32)sizeof(u32);
	Hdr->OpHdr.Col = _XAie_GetColfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Row = _XAie_GetRowfromRegOff(DevInst,Cmd->RegOff);
	Hdr->OpHdr.Op = (u8)XAIE_IO_BLOCKWRITE;

	for (u32 i = 0U; i < Cmd->Size; i++) {
		*((u32 *)Payload) = Cmd->Value;
		Payload += 4;
	}
}

static inline void _XAie_AppendCustomOp(XAie_TxnCmd *Cmd, u8 *TxnPtr)
{
	u8 *Payload = TxnPtr + sizeof(XAie_CustomOpHdr);
	XAie_CustomOpHdr *Hdr = (XAie_CustomOpHdr*)TxnPtr;

	Hdr->Size = (u32)sizeof(*Hdr) + Cmd->Size * (u32)sizeof(u8);
	Hdr->OpHdr.Op = (u8)Cmd->Opcode;

	for (u32 i = 0U; i < Cmd->Size; ++i, ++Payload) {
		*(Payload) = *((u8*)(uintptr_t)Cmd->DataPtr + i);
	}
}

static u8* _XAie_ReallocTxnBuf(u8 *TxnPtr, u32 NewSize)
{
	u8 *Tmp;
	Tmp =  (u8*)realloc((void*)TxnPtr, NewSize);
	if(Tmp == NULL) {
		XAIE_ERROR("Reallocation failed for txn buffer\n");
		return NULL;
	}

	return Tmp;
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
	XAie_TxnInst *TmpInst;
	u8 *TxnPtr;
	u32 BuffSize = 0U, NumOps = 0;
	u32 AllocatedBuffSize = XAIE_DEFAULT_TXN_BUFFER_SIZE;
	(void)NumConsumers;
	(void)Flags;

	TmpInst = _XAie_GetTxnInst(DevInst, Backend->Ops.GetTid());
	if(TmpInst == NULL) {
		XAIE_ERROR("Failed to get the correct transaction instance "
				"from internal list\n");
		return NULL;
	}

	TxnPtr = malloc(AllocatedBuffSize);
	if(TxnPtr == NULL) {
		XAIE_ERROR("Failed to allocate memory for alocated buffer\n");
		return NULL;
	}

	_XAie_CreateTxnHeader(DevInst, (XAie_TxnHeader *)TxnPtr);
	BuffSize += (u32)sizeof(XAie_TxnHeader);
	TxnPtr += sizeof(XAie_TxnHeader);
	XAIE_DBG("number of cmd %d\n", TmpInst->NumCmds);

	for(u32 i = 0U; i < TmpInst->NumCmds; i++) {
		NumOps++;
		XAie_TxnCmd *Cmd = &TmpInst->CmdBuf[i];
		if ((Cmd->Opcode == XAIE_IO_WRITE) && (Cmd->Mask == 0U)) {
			if((BuffSize + sizeof(XAie_Write32Hdr)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U);
				if(TxnPtr == NULL){
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
		if ((Cmd->Opcode == XAIE_IO_WRITE) && ((Cmd->Mask)!=0U)) {
			if((BuffSize + sizeof(XAie_MaskWrite32Hdr)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U);
				if(TxnPtr == NULL) {
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
		if (Cmd->Opcode == XAIE_IO_MASKPOLL) {
			if((BuffSize + sizeof(XAie_MaskPoll32Hdr)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U);
				if(TxnPtr == NULL) {
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
		if (Cmd->Opcode == XAIE_IO_BLOCKWRITE) {
			/**
			 * In case of Block Write and Block Set, it is possible
			 * that the new allocated buffer size may not be sufficient.
			 * In that case we should keep reallocating till the new
			 * buffer size if big enough to hold existing + current opcode.
			 */
			while((BuffSize + sizeof(XAie_BlockWrite32Hdr) +
						Cmd->Size * sizeof(u32)) >
					AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U);
				if(TxnPtr == NULL) {
					return NULL;
				}
				AllocatedBuffSize *= 2U;
				TxnPtr += BuffSize;
			}
			_XAie_AppendBlockWrite32(DevInst, Cmd, TxnPtr);
			TxnPtr += sizeof(XAie_BlockWrite32Hdr) +
				Cmd->Size * sizeof(u32);
			BuffSize += (u32)sizeof(XAie_BlockWrite32Hdr) +
				Cmd->Size * (u32)sizeof(u32);
			continue;
		}
		if (Cmd->Opcode == XAIE_IO_BLOCKSET) {
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
				TxnPtr = _XAie_ReallocTxnBuf(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U);
				if(TxnPtr == NULL) {
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
		if (Cmd->Opcode >= XAIE_IO_CUSTOM_OP_BEGIN) {
			if (TX_DUMP_ENABLE) {
				TxnCmdDump(Cmd);
			}

			XAIE_DBG("Size of the CustomOp Hdr being exported: %u bytes\n", sizeof(XAie_CustomOpHdr));

			if((BuffSize + sizeof(XAie_CustomOpHdr) +
						Cmd->Size) > AllocatedBuffSize) {
				TxnPtr = _XAie_ReallocTxnBuf(TxnPtr - BuffSize,
						AllocatedBuffSize * 2U);
				if(TxnPtr == NULL) {
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

	BuffSize = ((BuffSize % 4U) != 0U) ? (BuffSize / 4U + 1U) : BuffSize;
	XAIE_DBG("Size of the Txn Hdr being exported: %u bytes\n",
			sizeof(XAie_TxnHeader));
	XAIE_DBG("Size of the transaction buffer being exported: %u bytes\n",
			BuffSize);
	XAIE_DBG("Num of Operations in the transaction buffer: %u\n",
			TmpInst->NumCmds);


	/* Adjust pointer and reallocate to the right size */
	TxnPtr = _XAie_ReallocTxnBuf(TxnPtr - BuffSize, BuffSize);
	if(TxnPtr == NULL) {
		XAIE_ERROR("TxnPtr realloc failed\n");
		return NULL;
	}
	((XAie_TxnHeader *)TxnPtr)->NumOps =  NumOps;
	((XAie_TxnHeader *)TxnPtr)->TxnSize =  BuffSize;

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
		if((Cmd->Opcode == XAIE_IO_BLOCKWRITE || Cmd->Opcode >= XAIE_IO_CUSTOM_OP_BEGIN) &&
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
		TxnInst = XAIE_CONTAINER_OF(NodePtr, XAie_TxnInst, Node);

		if (TxnInst == NULL) {
			continue;
		}

		for(u32 i = 0; i < TxnInst->NumCmds; i++) {
			XAie_TxnCmd *Cmd = &TxnInst->CmdBuf[i];
			//TBD handle custom OP as well
			if((Cmd->Opcode == XAIE_IO_BLOCKWRITE || Cmd->Opcode >= XAIE_IO_CUSTOM_OP_BEGIN) &&
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
* This API waits for a specified number of asynchronous operations to complete.
*
* @param	DevInst: Device Instance
* @param	Nr: Number of asynchronous operations to wait for.
*
* @return	XAIE_OK on success, XAIE_INVALID_DEVICE if async wait is not
*		supported by the backend.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_AsyncWaitNr(XAie_DevInst *DevInst, u32 Nr)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (Backend->Ops.AsyncWait != NULL) {
		return Backend->Ops.AsyncWaitNr((void*)(DevInst->IOInst), Nr);
	} else {
		XAIE_ERROR("Asynchronous wait operation is not supported "
				"by the backend\n");
		return XAIE_INVALID_DEVICE;
	}
}

/*****************************************************************************/
/**
*
* This API waits for first pending asynchronous operations to complete.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success, XAIE_INVALID_DEVICE if async wait is not
*		supported by the backend.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_AsyncWait(XAie_DevInst *DevInst)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (Backend->Ops.AsyncWait != NULL) {
		return Backend->Ops.AsyncWait((void*)(DevInst->IOInst));
	} else {
		XAIE_ERROR("Asynchronous wait operation is not supported "
				"by the backend\n");
		return XAIE_INVALID_DEVICE;
	}
}

/*****************************************************************************/
/**
*
* This API asynchronously writes a 32-bit value to the specified register
* offset. It validates the input parameters and delegates to the backend's
* Write32Async operation.
*
* @param	DevInst: Device Instance
* @param	RegOff: Register offset to write to.
* @param	Value: 32-bit value to be written.
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking. Must not be NULL. On error, AsyncRes->res
*			is set to the corresponding error code.
*
* @return	Number of SQEs submitted on success, or 0 on failure.
*
* @note		None.
*
******************************************************************************/
int XAie_Write32Async(XAie_DevInst *DevInst, u64 RegOff, u32 Value, XAie_AsyncRes *AsyncRes)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (AsyncRes == NULL) {
		XAIE_ERROR("AsyncRes pointer cannot be NULL\n");
		return 0;
	}
	if (Backend->Ops.Write32Async == NULL) {
		XAIE_ERROR("Asynchronous write operation is not supported "
				"by the backend\n");
		AsyncRes->res = XAIE_INVALID_DEVICE;
		return 0;
	}

	return Backend->Ops.Write32Async((void*)(DevInst->IOInst), RegOff,
					 Value, AsyncRes);
}

AieRC XAie_Write32(XAie_DevInst *DevInst, u64 RegOff, u32 Value)
{
	u64 Tid;
	AieRC RC;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
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
	return Backend->Ops.Write32((void*)(DevInst->IOInst), RegOff, Value);
}

AieRC XAie_Read32(XAie_DevInst *DevInst, u64 RegOff, u32 *Data)
{
	u64 Tid;
	AieRC RC;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
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
	return Backend->Ops.Read32((void*)(DevInst->IOInst), RegOff, Data);
}

AieRC XAie_MaskWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
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
	return Backend->Ops.MaskWrite32((void *)(DevInst->IOInst), RegOff, Mask,
			Value);
}

int XAie_MaskWrite32Async(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
			  XAie_AsyncRes *AsyncRes)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (AsyncRes == NULL) {
		XAIE_ERROR("AsyncRes pointer cannot be NULL\n");
		return 0;
	}
	if (Backend->Ops.MaskWrite32Async == NULL) {
		XAIE_ERROR("Asynchronous mask write operation is not supported "
				"by the backend\n");
		AsyncRes->res = XAIE_INVALID_DEVICE;
		return 0;
	}

	return Backend->Ops.MaskWrite32Async((void *)(DevInst->IOInst), RegOff, Mask,
					Value, AsyncRes);
}

AieRC XAie_MaskPoll(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
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
	return Backend->Ops.MaskPoll((void*)(DevInst->IOInst), RegOff, Mask,
			Value, TimeOutUs);
}

/*****************************************************************************/
/**
*
* This function performs an asynchronous 64-byte block write to the AIE device.
* It validates the input parameters and delegates to the backend's
* BlockWrite64BytesAsync operation.
*
* @param	DevInst: Device Instance
* @param	RegOff: Register offset to write to.
* @param	Data: Pointer to data buffer containing u32 words to write.
* @param	Size: Number of u32 words to write.
* @param	AsyncRes: Pointer to async result structure for completion
*			  tracking. Must not be NULL.
*
* @return	Number of SQEs submitted on success, or 0 on failure.
*
* @note		None.
*
******************************************************************************/
int XAie_BlockWrite64Async(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data,
			   u32 Size, XAie_AsyncRes *AsyncRes)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (AsyncRes == NULL) {
		XAIE_ERROR("AsyncRes pointer cannot be NULL\n");
		return 0;
	}
	if (Backend->Ops.BlockWrite64BytesAsync == NULL) {
		XAIE_ERROR("Asynchronous block write operation is not supported "
				"by the backend\n");
		AsyncRes->res = XAIE_INVALID_DEVICE;
		return 0;
	}

	return Backend->Ops.BlockWrite64BytesAsync((void *)(DevInst->IOInst), RegOff,
						   Data, Size, AsyncRes);
}

/*****************************************************************************/
/**
*
* This function performs an asynchronous 32-bit block write to the AIE device.
* It validates the input parameters and delegates to the backend's
* BlockWrite32Async operation. PM and DM memory writes are not supported.
*
* @param	DevInst: Device Instance
* @param	RegOff: Register offset to write to.
* @param	Data: Pointer to data buffer containing u32 words to write.
* @param	Size: Number of u32 words to write.
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking. Must not be NULL. On error, AsyncRes->res
*			is set to the corresponding error code.
*
* @return	Number of SQEs submitted on success, or 0 on failure.
*
* @note		Does not support PM/DM memory writes.
*
******************************************************************************/
int XAie_BlockWrite32Async(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data,
			   u32 Size, XAie_AsyncRes *AsyncRes)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (AsyncRes == NULL) {
		XAIE_ERROR("AsyncRes pointer cannot be NULL\n");
		return 0;
	}
	if (Backend->Ops.BlockWrite32Async == NULL) {
		XAIE_ERROR("Asynchronous block write32 operation is not "
				"supported by the backend\n");
		AsyncRes->res = XAIE_INVALID_DEVICE;
		return 0;
	}

	return Backend->Ops.BlockWrite32Async((void *)(DevInst->IOInst), RegOff,
					      Data, Size, AsyncRes);
}

AieRC XAie_BlockWrite32(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data, u32 Size)
{
	AieRC RC;
	u64 Tid;
	u32 *Buf;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
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

		Buf = memcpy((void *)Buf, (void *)Data, sizeof(u32) * Size);
		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = XAIE_IO_BLOCKWRITE;
		TxnInst->CmdBuf[TxnInst->NumCmds].RegOff = RegOff;
		TxnInst->CmdBuf[TxnInst->NumCmds].DataPtr = (u64)(uintptr_t)Buf;
		TxnInst->CmdBuf[TxnInst->NumCmds].Size = Size;
		TxnInst->CmdBuf[TxnInst->NumCmds].Mask = 0U;
		TxnInst->NumCmds++;

		return XAIE_OK;
	}
	return Backend->Ops.BlockWrite32((void *)(DevInst->IOInst), RegOff,
			Data, Size);
}

AieRC XAie_BlockSet32(XAie_DevInst *DevInst, u64 RegOff, u32 Data, u32 Size)
{
	AieRC RC;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
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
	return Backend->Ops.BlockSet32((void *)(DevInst->IOInst), RegOff, Data,
			Size);
}

/*****************************************************************************/
/**
*
* This function performs an asynchronous 32-bit block set on the AIE device.
* It validates the input parameters and delegates to the backend's
* BlockSet32Async operation. PM and DM memory regions are not supported.
*
* @param	DevInst: Device Instance
* @param	RegOff: Register offset to write to.
* @param	Data: 32-bit value to fill the address range with.
* @param	Size: Number of 32-bit words to set.
* @param	AsyncRes: Pointer to async result structure for completion
*			tracking. Must not be NULL. On error, AsyncRes->res
*			is set to the corresponding error code.
*
* @return	Number of SQEs submitted on success, or 0 on failure.
*
* @note		Does not support PM/DM memory regions.
*
******************************************************************************/
int XAie_BlockSet32Async(XAie_DevInst *DevInst, u64 RegOff, u32 Data,
			 u32 Size, XAie_AsyncRes *AsyncRes)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (AsyncRes == NULL) {
		XAIE_ERROR("AsyncRes pointer cannot be NULL\n");
		return 0;
	}
	if (Backend->Ops.BlockSet32Async == NULL) {
		XAIE_ERROR("Asynchronous block set32 operation is not "
				"supported by the backend\n");
		AsyncRes->res = XAIE_INVALID_DEVICE;
		return 0;
	}

	return Backend->Ops.BlockSet32Async((void *)(DevInst->IOInst), RegOff,
					    Data, Size, AsyncRes);
}

AieRC XAie_CmdWrite(XAie_DevInst *DevInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	const XAie_Backend *Backend = DevInst->Backend;
	AieRC result = XAIE_OK;

	if(DevInst->TxnList.Next != NULL) {
		/**
		 * This function is only used in XAie_LoadElf() on AIESIM platform.
		 * In the past the elf loading was done via XCLBIN (PDI) but not
		 * via TXN binary. Hence this unwanted TXN implementation did not have
		 * any side effects. But when elf loading is attempted via TXN flow
		 * this needs to be made a NOOP else it fails. Hence Making
		 * XAIe_CmdWrite no-op for transaction mode.
		 **/
		result = XAIE_OK;
	} else {
		result = Backend->Ops.CmdWrite((void *)(DevInst->IOInst), Col, Row,
				Command, CmdWd0, CmdWd1, CmdStr);
	}
	return result;
}

AieRC XAie_RunOp(XAie_DevInst *DevInst, XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC;
	void *Buf;
	u64 Tid;
	XAie_TxnInst *TxnInst;
	const XAie_Backend *Backend = DevInst->Backend;

	if(DevInst->TxnList.Next != NULL) {
		Tid = Backend->Ops.GetTid();
		TxnInst = _XAie_GetTxnInst(DevInst, Tid);
		if(TxnInst == NULL) {
			XAIE_DBG("Could not find transaction instance "
					"associated with thread. Running Op.\n");
			return Backend->Ops.RunOp(DevInst->IOInst, DevInst, Op, Arg);
		}

		if (Op == XAIE_BACKEND_OP_CONFIG_SHIMDMABD) {
			XAie_ShimDmaBdArgs *BdArgs =
				(XAie_ShimDmaBdArgs *)Arg;
			if (Backend->Type == XAIE_IO_BACKEND_LINUX) {
				if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
					RC = _XAie_ReallocCmdBuf(TxnInst);
					if (RC != XAIE_OK) {
						return RC;
					}
				}

				Buf = Backend->Ops.GetShimDmaBdConfig(Arg);
				if (Buf == XAIE_NULL) {
					XAIE_ERROR("Could not get SHIM DMA structure\n");
					return XAIE_ERR;
				}

				if (BdArgs->MemInst == XAIE_NULL) {
					TxnInst->CmdBuf[TxnInst->NumCmds].Opcode =
							XAIE_CONFIG_SHIMDMA_BD;
				} else {
					TxnInst->CmdBuf[TxnInst->NumCmds].Opcode =
							XAIE_CONFIG_SHIMDMA_DMABUF_BD;
				}

				TxnInst->CmdBuf[TxnInst->NumCmds].DataPtr =
								(u64)(uintptr_t)Buf;
				TxnInst->NumCmds++;
			} else {
				XAie_BlockWrite32(DevInst,
						     BdArgs->Addr, BdArgs->BdWords,
						     BdArgs->NumBdWords);
			}
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
			return Backend->Ops.RunOp(DevInst->IOInst, DevInst, Op, Arg);
		} else if(TxnInst->NumCmds == 0U) {
			return Backend->Ops.RunOp(DevInst->IOInst, DevInst, Op, Arg);
		} else {
			XAIE_ERROR("Run Op operation is not supported "
					"when auto flush is disabled\n");
			return XAIE_ERR;
		}
	}
	return Backend->Ops.RunOp(DevInst->IOInst, DevInst, Op, Arg);
}

AieRC XAie_AddressPatching(XAie_DevInst *DevInst, u32 Arg_Offset, u8 Num_BDs)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (Backend->Ops.AddressPatching != NULL) {
		return Backend->Ops.AddressPatching((void *)DevInst->IOInst, Arg_Offset, Num_BDs);
	} else {
		XAIE_ERROR("Address Patching function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}

}

AieRC XAie_WaitUCDMA(XAie_DevInst *DevInst)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (Backend->Ops.WaitUcDMA != NULL) {
		return Backend->Ops.WaitUcDMA((void *)DevInst->IOInst);
	} else {
		XAIE_ERROR("WaitUCDMA function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

AieRC XAie_ModeConfig(XAie_DevInst *DevInst, XAie_ModeSelect Mode)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (Backend->Ops.ConfigMode != NULL) {
		return Backend->Ops.ConfigMode((void *)DevInst->IOInst, Mode);
	} else {
		XAIE_ERROR("ConfigMode function pointer points to NULL\n");
		return XAIE_NOT_SUPPORTED;
	}
}

XAie_ModeSelect XAie_GetModeConfig(XAie_DevInst *DevInst)
{
	const XAie_Backend *Backend = DevInst->Backend;

	if (Backend->Ops.GetConfigMode != NULL) {
		return Backend->Ops.GetConfigMode((void *)DevInst->IOInst);
	} else {
		XAIE_ERROR("GetConfigMode function pointer points to NULL, hence returned mode is XAIE_INVALID_MODE\n");
		return XAIE_INVALID_MODE;
	}
}

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
		if(Cmd->Opcode == XAIE_IO_BLOCKWRITE || Cmd->Opcode >= XAIE_IO_CUSTOM_OP_BEGIN) {
			XAIE_DBG("free DataPtr %p\n", (void *)(uintptr_t)Cmd->DataPtr);
			free((void *)(uintptr_t)Cmd->DataPtr);
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

		if ( TxnInst->NextCustomOp < OpNumber || (u8)XAIE_IO_CUSTOM_OP_BEGIN > OpNumber) {
			XAIE_ERROR("Invalid Op Code %d\n",OpNumber);
			return XAIE_ERR;
		}

		/* check memory allocation before increase Cmd vector */
		char* tmpBuff = malloc(size);
		if(tmpBuff == XAIE_NULL) {
			XAIE_ERROR("Fail to malloc %d size memory for DataPtr\n", size);
			return XAIE_ERR;
		}

		if(TxnInst->NumCmds + 1U == TxnInst->MaxCmds) {
			RC = _XAie_ReallocCmdBuf(TxnInst);
			if (RC != XAIE_OK) {
				 return RC;
			}

		}

		TxnInst->CmdBuf[TxnInst->NumCmds].Opcode = (XAie_TxnOpcode)OpNumber;
		TxnInst->CmdBuf[TxnInst->NumCmds].Size = (u32)size;

		memcpy(tmpBuff, Args, size);
		TxnInst->CmdBuf[TxnInst->NumCmds].DataPtr = (u64)(uintptr_t)tmpBuff;

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
		XAIE_ERROR("Invalid Tile Type, Col:%u Row:%u TileType:%u\n", Loc.Col, Loc.Row, TileType);
		return XAIE_INVALID_TILE;
	}

	/* core status */
	RC = XAie_CoreGetStatus(DevInst, Loc, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}
	Index = Loc.Row - TileStart;
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
		XAIE_ERROR("Invalid Tile Type, Col:%u Row:%u TileType:%u\n", Loc.Col, Loc.Row, TileType);
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
		XAIE_ERROR("Invalid Tile Type, Col:%u Row:%u TileType:%u\n", Loc.Col, Loc.Row, TileType);
		return XAIE_INVALID_TILE;
	}
	LockMod = DevInst->DevProp.DevMod[TileType].LockMod;
	CoreTile = Status[Loc.Col].CoreTile;
	MemTile = Status[Loc.Col].MemTile;
	ShimTile = Status[Loc.Col].ShimTile;

	/* iterate all lock value registers */
	for(u32 LockCnt = 0; LockCnt < LockMod->NumLocks; LockCnt++) {

		/* read lock value */
		Lock.LockId = (u16)LockCnt;
		RC = XAie_LockGetValue(DevInst, Loc, Lock, &RegVal);
		if (RC != XAIE_OK) {
			return RC;
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
	XAie_TileMod *DevMod;

	AieTileStart = DevInst->AieTileRowStart;
	MemTileStart = DevInst->MemTileRowStart;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType >= XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid Tile Type, Col:%u Row:%u TileType:%u\n", Loc.Col, Loc.Row, TileType);
		return XAIE_INVALID_TILE;
	}

	CoreTile = Status[Loc.Col].CoreTile;
	MemTile = Status[Loc.Col].MemTile;
	ShimTile = Status[Loc.Col].ShimTile;
	DevMod = DevInst->DevProp.DevMod;

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

AieRC XAie_PrintDmaStatus(XAie_DevInst *DevInst, XAie_LocType Loc, const XAie_DmaMod *DmaMod)
{
	u32 s2mm_val, mm2s_val;
	AieRC RC;

        for (u8 Chan = 0; Chan < DmaMod->NumChannels; Chan++) {
			RC = XAie_DmaGetChannelStatus(DevInst, Loc, Chan, DMA_S2MM, &s2mm_val);
			if (RC != XAIE_OK)
					return RC;

			RC = XAie_DmaGetChannelStatus(DevInst, Loc, Chan, DMA_MM2S, &mm2s_val);
			if (RC != XAIE_OK)
					return RC;
			XAIE_DBG("Tile[%u,%u] Chan[%d]: S2MM=0x%x | MM2S=0x%x\n", Loc.Col, Loc.Row, Chan, s2mm_val, mm2s_val);
        }

        return XAIE_OK;
}

AieRC XAie_PrintBdStatus(XAie_DevInst *DevInst, XAie_LocType Loc, const XAie_DmaMod *DmaMod, u8 BdWordCount)
{
	u64 BdAddr;
	u32 bd_val;
	AieRC RC;

		for(u8 BD=0; BD < DmaMod->NumBds; BD++){
			BdAddr = (u64)DmaMod->BaseAddr +
						(u64)XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + (u64)(BD * DmaMod->IdxOffset);
			for(u8 i = 0; i < BdWordCount; i++) {
				RC = XAie_Read32(DevInst, BdAddr, &bd_val);
				if (RC != XAIE_OK)
						return RC;

				XAIE_DBG("BD_%d_%d: 0x%x \n", BD, i, bd_val);
				BdAddr += 4U;
			}
			printf("\n");
		}
        return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API returns the column status for N number of colums.
*
* @param        DevInst: Device Instance
* @param        StartCol: Start column
* @param        NumCols: Number of columns
*
* @return       XAIE_OK for success and error code otherwise.
*
* @note None.
*
******************************************************************************/
AieRC XAie_DmaStatusDump(XAie_DevInst *DevInst, u8 StartCol, u8 NumCols)
{
        u8 NumRows = DevInst->NumRows;
        u32 RC = (u32)XAIE_ERR;

        for (u8 Col = StartCol; Col < (StartCol + NumCols); Col++) {
                for (u8 Row = 0; Row < NumRows; Row++) {
                        XAie_LocType Loc = {.Col = Col, .Row = Row};
                        u8 TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

                        if (TileType >= XAIEGBL_TILE_TYPE_MAX || TileType == XAIEGBL_TILE_TYPE_SHIMPL)
                                continue;

                        const XAie_DmaMod *DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
                        if (DmaMod == NULL)
                                return XAIE_ERR;

                        RC = XAie_PrintDmaStatus(DevInst, Loc, DmaMod);
                        if (RC != XAIE_OK)
                                return RC;

			switch (TileType) {
				case XAIEGBL_TILE_TYPE_AIETILE:
					if (DevInst->DevProp.DevGen == 1)
						RC = XAie_PrintBdStatus(DevInst, Loc, DmaMod, XAIE_TILEDMA_NUM_BD_WORDS);
					else if (DevInst->DevProp.DevGen == 2)
						RC = XAie_PrintBdStatus(DevInst, Loc, DmaMod, XAIEML_TILEDMA_NUM_BD_WORDS);
					else if (DevInst->DevProp.DevGen == 5)
						RC = XAie_PrintBdStatus(DevInst, Loc, DmaMod, XAIE2PS_TILEDMA_NUM_BD_WORDS);
					break;
				case XAIEGBL_TILE_TYPE_MEMTILE:
					if (DevInst->DevProp.DevGen == 2)
						RC = XAie_PrintBdStatus(DevInst, Loc, DmaMod, XAIEML_MEMTILEDMA_NUM_BD_WORDS);
					else if (DevInst->DevProp.DevGen == 5)
						RC = XAie_PrintBdStatus(DevInst, Loc, DmaMod, XAIE2PS_MEMTILEDMA_NUM_BD_WORDS);
					break;
				case XAIEGBL_TILE_TYPE_SHIMNOC:
					if (DevInst->DevProp.DevGen == 1)
						RC = XAie_PrintBdStatus(DevInst, Loc, DmaMod, XAIE_SHIMDMA_NUM_BD_WORDS);
					else if (DevInst->DevProp.DevGen == 2)
						RC = XAie_PrintBdStatus(DevInst, Loc, DmaMod, XAIEML_SHIMDMA_NUM_BD_WORDS);
					else if (DevInst->DevProp.DevGen == 5)
						RC = XAie_PrintBdStatus(DevInst, Loc, DmaMod, XAIE2PS_SHIMDMA_NUM_BD_WORDS);
					break;
			}

			if (RC != XAIE_OK)
				return RC;
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
	u32 RC = (u32)XAIE_ERR;
	u8 StartCol = DevInst->StartCol;
	u8 NumCols  = DevInst->NumCols;
	u8 NumRows  = DevInst->NumRows;
	XAie_LocType Loc;
	u8 TileType;

	if(Status == NULL) {
		return XAIE_ERR;
	}

	/* iterate specified columns */
	for(u8 Col = StartCol; Col < NumCols; Col++) {
		for(u8 Row = 0; Row < NumRows; Row++) {
			Loc.Row = Row;
			Loc.Col = Col;
			TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
			if(TileType >= XAIEGBL_TILE_TYPE_MAX) {
				continue;
			}

			RC |= (u32)_XAie_EventStatusDump(DevInst, Status, Loc);

			if(TileType == XAIEGBL_TILE_TYPE_AIETILE) {
				RC |= (u32)_XAie_CoreStatusDump(DevInst,Status, Loc);
			} else if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
				continue;
			}

			RC |= (u32)_XAie_DmaStatusDump(DevInst, Status, Loc);
			RC |= (u32)_XAie_LockValueStatusDump(DevInst, Status, Loc);
		}
	}
	return (AieRC)RC;
}

/*****************************************************************************/
/**
*
* This routine is used to check if the shim tile has uc module.
*
* @param	DevInst: Device Instance.
* @param	TileType: Type of the tile.
*
* @return	1 if uc module is present and 0 otherwise.
*
*******************************************************************************/
u8 XAie_IsUcModulePresent(XAie_DevInst *DevInst, u8 TileType)
{
	if(DevInst->DevProp.DevGen >= XAIE_DEV_GEN_AIE2PS &&
			TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
		return DevInst->DevProp.DevMod[TileType].UcMod == XAIE_NULL ? 0 : 1;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This API checks the SHIM uC modules memory privileged bit
*
* @param	DevInst: Device Instance
* @param	Loc: Location of SHIM Tile
* @param	Priv: Return for status of privileged bit
*
* @return	XAIE_OK for success, error code for failure
*
* @note		Internal API only
******************************************************************************/
AieRC _XAie_IsUcPrivilegedSet(XAie_DevInst *DevInst, XAie_LocType Loc, u8 *Priv)
{
	AieRC RC;
	u32 RegVal;
	u64 Offset;
	u8 TileType;
	const XAie_UcMod *UcMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(XAie_IsUcModulePresent(DevInst, TileType) == 0U) {
		XAIE_ERROR("Tile does not have uC module. Col:%u Row:%u TileType:%u\n",Loc.Col, Loc.Row, TileType);
		return XAIE_INVALID_TILE;
	}

	UcMod = DevInst->DevProp.DevMod[TileType].UcMod;
	Offset = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		UcMod->MemPrivilegedOffset;
	RC = XAie_Read32(DevInst, Offset, &RegVal);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Privileged memory register. Failed to read register value. Offset: %lu\n", Offset);
		return RC;
	}

	if((RegVal & 0x1) != 0) {
		*Priv = 1U;
	}
	*Priv = 0U;

	return RC;
}

AieRC XAie_SetPadInteger(XAie_DevInst *DevInst, char* BuffName, u32 BuffSize)
{
    const XAie_Backend *Backend = DevInst->Backend;

    if (Backend->Ops.SetPadInteger != NULL) {
        return Backend->Ops.SetPadInteger((void *)DevInst->IOInst, BuffName, BuffSize);
    } else {
        XAIE_ERROR("SetPadInteger function pointer points to NULL\n");
        return XAIE_INVALID_ARGS;
    }
}

AieRC XAie_SetPadString(XAie_DevInst *DevInst, char* BuffName, char* BuffBlobPath)
{
    const XAie_Backend *Backend = DevInst->Backend;

    if (Backend->Ops.SetPadString != NULL) {
        return Backend->Ops.SetPadString((void *)DevInst->IOInst, BuffName, BuffBlobPath);
    } else {
        XAIE_ERROR("SetPadString function pointer points to NULL\n");
        return XAIE_INVALID_ARGS;
    }
}

/** @} */
