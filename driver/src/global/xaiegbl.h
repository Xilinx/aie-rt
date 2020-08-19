/******************************************************************************
*
* Copyright (C) 2005 - 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaiegbl.h
* @{
*
* This file contains the instances of the register bit field definitions for the
* Core, Memory, NoC and PL module registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   10/28/2019  Add error type for pl interface apis
* 1.2   Tejus   12/09/2019  Forward declaration of structures
* 1.3   Tejus   03/17/2020  Add error types and data structure for lock apis
* 1.4   Tejus   03/21/2020  Add data strcuture and initialization function for
*			    packets.
* 1.5   Tejus   03/22/2020  Remove data strcutures from initial dma api
*			    implementation
* 1.6   Tejus   03/23/2020  Add data structures for common dma apis.
* 1.7   Tejus   04/13/2020  Remove range structure and helper function.
* 1.8   Dishita 04/27/2020  Add enum for reset and modules.
* </pre>
*
******************************************************************************/

#ifndef XAIEGBL_H /* prevent circular inclusions */
#define XAIEGBL_H /* by using protection macros */

/***************************** Include Files *********************************/
#include "xaiegbl_defs.h"

/************************** Constant Definitions *****************************/
#define XAIE_LOCK_WITH_NO_VALUE	(-1)
/**************************** Type Definitions *******************************/
typedef struct XAie_TileMod XAie_TileMod;
typedef struct XAie_DmaMod XAie_DmaMod;
typedef struct XAie_LockMod XAie_LockMod;

/*
 * This typedef captures all the properties of a AIE Device
 */
typedef struct XAie_DevProp {
	u8 DevGen;
	u8 RowShift;
	u8 ColShift;
	XAie_TileMod *DevMod;
} XAie_DevProp;

/*
 * This typedef contains the attributes for a AIE partition. The structure is
 * setup during intialization.
 */
typedef struct {
	u64 BaseAddr; /* Base address of the partition*/
	u8 NumRows;   /* Number of rows allocated to the partition */
	u8 NumCols;   /* Number of cols allocated to the partition */
	u8 ShimRow;   /* ShimRow location */
	u8 MemTileRowStart; /* Mem tile starting row in the partition */
	u8 MemTileNumRows;  /* Number of memtile rows in the partition */
	u8 AieTileRowStart; /* Aie tile starting row in the partition */
	u8 AieTileNumRows;  /* Number of aie tile rows in the partition */
	u8 IsReady;
	XAie_DevProp DevProp; /* Pointer to the device property. To be
				     setup to AIE prop during intialization*/
} XAie_DevInst;

typedef struct {
	u8 AieGen;
	u64 BaseAddr;
	u8 ColShift;
	u8 RowShift;
	u8 NumRows;
	u8 NumCols;
	u8 ShimRowNum;
	u8 MemTileRowStart;
	u8 MemTileNumRows;
	u8 AieTileRowStart;
	u8 AieTileNumRows;
} XAie_Config;

/*
 * This typedef contains attributes for a tile coordinate.
 */
typedef struct {
	u8 Row;
	u8 Col;
} XAie_LocType;

/*
 * This enum contains all the Stream Switch Port types. These enums are used to
 * access the base address of stream switch configuration registers.
 */
typedef enum{
	CORE,
	DMA,
	CTRL,
	FIFO,
	SOUTH,
	WEST,
	NORTH,
	EAST,
	TRACE,
	SS_PORT_TYPE_MAX
} StrmSwPortType;

/* Data structures to capture data shape for dmas */
typedef struct {
	u32 StepSize;
	u32 Wrap;
} XAie_Gen2DmaDimDesc;

typedef struct {
	u32 Offset;
	u32 Incr;
	u32 Wrap;
} XAie_Gen1DmaDimDesc;

typedef union {
	XAie_Gen1DmaDimDesc AieDimDesc;
	XAie_Gen2DmaDimDesc Aie2DimDesc;
} XAie_DmaDimDisc;

typedef struct {
	u8 NumDim;
	XAie_DmaDimDisc Dim[];
} XAie_DmaTensor;

typedef struct {
	u8 LockAcqId;
	u8 LockRelId;
	u8 LockAcqEn;
	s8 LockAcqVal;
	u8 LockAcqValEn;
	u8 LockRelEn;
	s8 LockRelVal;
	u8 LockRelValEn;
} XAie_LockDesc;

typedef struct {
	u8 PktId;
	u8 PktType;
	u8 PktEn;
} XAie_PktDesc;

typedef struct {
	u64 Address;
	u32 Length;
} XAie_AddrDesc;

typedef struct {
	u8 ValidBd;
	u8 NxtBd;
	u8 UseNxtBd;
	u8 OutofOrderBdId;
} XAie_BdEnDesc;

typedef struct {
	u8 SMID;
	u8 BurstLen;
	u8 AxQos;
	u8 SecureAccess;
	u8 AxCache;
} XAie_DmaAxiDesc;

typedef struct {
	u8 X_Incr;
	u8 X_Wrap;
	u16 X_Offset;
	u8 Y_Incr;
	u8 Y_Wrap;
	u16 Y_Offset;
	u8 IntrleaveBufSelect;
	u16 CurrPtr;
	u8 IntrleaveCount;
	u8 EnInterleaved;
} XAie_Gen1MultiDimDesc;

typedef struct {
	u16 Wrap;
	u32 StepSize;
} XAie_Gen2DimDesc;

typedef struct {
	u8 IterCurr;
	XAie_Gen2DimDesc IterDesc;
	XAie_Gen2DimDesc DimDesc[4U];	/* Max 4D addressing supported */
} XAie_Gen2MultiDimDesc;

typedef union {
	XAie_Gen1MultiDimDesc Gen1MultiDimDesc;
	XAie_Gen2MultiDimDesc Gen2MultiDimDesc;
} XAie_MultiDimDesc;

typedef struct {
	u8 D0_ZeroBefore;
	u8 D0_ZeroAfter;
	u8 D1_ZeroBefore;
	u8 D1_ZeroAfter;
	u8 D2_ZeroBefore;
	u8 D2_ZeroAfter;
} XAie_ZeroPadDesc;

typedef struct {
	u8 ControllerId;
	u8 EnOutofOrderId;
	u8 Reset;
	u8 EnTokenIssue;
	u8 PauseStream;
	u8 PauseMem;
	u8 Enable;
} XAie_ChannelDesc;

typedef struct {
	XAie_PktDesc PktDesc;
	XAie_LockDesc LockDesc;
	XAie_AddrDesc AddrDesc;
	XAie_DmaAxiDesc AxiDesc;
	XAie_BdEnDesc BdEnDesc;
	XAie_LockDesc LockDesc_2;
	XAie_AddrDesc AddrDesc_2;
	XAie_MultiDimDesc MultiDimDesc;
	XAie_ZeroPadDesc ZeroPadDesc;
	XAie_ChannelDesc ChDesc;
	const XAie_DmaMod *DmaMod;
	const XAie_LockMod *LockMod;
	u8 EnDoubleBuff;
	u8 EnFifoMode;
	u8 EnCompression;
	u8 EnOutofOrderBdId;
	u8 TileType;
	u8 IsReady;
} XAie_DmaDesc;

/*
 * This enum contains the dma channel reset for aie dmas.
 */
typedef enum {
	DMA_CHANNEL_UNRESET,
	DMA_CHANNEL_RESET
} XAie_DmaChReset;
/*
 * This enum contains the dma direction for aie dmas.
 */
typedef enum {
	DMA_S2MM,
	DMA_MM2S,
	DMA_MAX
} XAie_DmaDirection;

/*
 * This enum captures all the error codes from the driver
 */
typedef enum{
	XAIE_OK,
	XAIE_ERR,
	XAIE_INVALID_DEVICE,
	XAIE_INVALID_RANGE,
	XAIE_INVALID_ARGS,
	XAIE_INVALID_TILE,
	XAIE_ERR_STREAM_PORT,
	XAIE_INVALID_DMA_TILE,
	XAIE_INVALID_BD_NUM,
	XAIE_ERR_OUTOFBOUND,
	XAIE_INVALID_DATA_MEM_ADDR,
	XAIE_INVALID_ELF,
	XAIE_CORE_STATUS_TIMEOUT,
	XAIE_INVALID_CHANNEL_NUM,
	XAIE_INVALID_LOCK,
	XAIE_INVALID_DMA_DIRECTION,
	XAIE_INVALID_PLIF_WIDTH,
	XAIE_INVALID_LOCK_ID,
	XAIE_INVALID_LOCK_VALUE,
	XAIE_LOCK_RESULT_FAILED,
	XAIE_INVALID_DMA_DESC,
	XAIE_INVALID_ADDRESS,
	XAIE_FEATURE_NOT_SUPPORTED,
	XAIE_INVALID_BURST_LENGTH,
	XAIE_ERR_MAX
} AieRC;

/*
 * This enum is to identify different hardware modules within a tile type.
 * An AIE tile can have memory or core module. A PL or Shim tile will have
 * Pl module. A mem tile will have memory module. Any hardware module
 * addition in future generations of AIE needs to be appended to this enum.
 */
typedef enum{
	XAIE_MEM_MOD,
	XAIE_CORE_MOD,
	XAIE_PL_MOD,
} XAie_ModuleType;

/* This enum contains reset input values. */
typedef enum {
	XAIE_RESETDISABLE,
	XAIE_RESETENABLE,
} XAie_Reset;

/* Data structure to capture lock id and value */
typedef struct {
	u8 LockId;
	s8 LockVal;
} XAie_Lock;

/* Data structure to capture packet id and packet type */
typedef struct {
	u8 PktId;
	u8 PktType;
} XAie_Packet;

/**************************** Function prototypes ***************************/
AieRC XAie_CfgInitialize(XAie_DevInst *InstPtr, XAie_Config *ConfigPtr);

/*****************************************************************************/
/*
*
* This API returns a structure of type XAie_Loc which captures the lock id and
* lock value.
*
* @param	Id: Lock id
* @param	Value: Lock value.
*
* @return	Lock: Lock strcuture initialized with Id and Value.
*
* @note		None.
*
******************************************************************************/
static inline XAie_Lock XAie_LockInit(u8 Id, s8 Value)
{
	XAie_Lock Lock = {Id, Value};
	return Lock;
}

/*****************************************************************************/
/*
*
* This API returns a structure of type XAie_Packet which captures the packet id
* and packet type. XAie_Packet can be used to configure the packet properties
* of aie dmas and stream switches. Packet ID determins the route between ports,
* and packet type is used to differentiate packets from the same source.
*
* @param	PktId: Packet id(5 bits)
* @param	PktType: Packet type(3 bits)
*
* @return	Pkt: Packet strcuture initialized with Id and Type.
*
* @note		None.
*
******************************************************************************/
static inline XAie_Packet XAie_PacketInit(u8 PktId, u8 PktType)
{
	XAie_Packet Pkt = {PktId, PktType};
	return Pkt;
}

/*****************************************************************************/
/*
*
* This API returns a structure of type XAie_LocType given a col and row index of
* AIE. All APIs use this structure to identify the coordinates of AIE tiles.
*
* @param	col: column index
* @param	row: row index
*
* @return	Loc: strcuture containing row and col index.
*
* @note		None.
*
******************************************************************************/
static inline XAie_LocType XAie_TileLoc(u8 col, u8 row)
{
	XAie_LocType Loc = { row, col };
	return Loc;
}

/*****************************************************************************/
/**
*
* Macro to setup the configurate pointer data structure with hardware specific
* details.
*
* @param	Config: XAie_Config structure.
* @param	_AieGEn: Aie device generation.
* @param	_BaseAddr: Base Address of the device.
* @param	_ColShift: Bit shift value for column.
* @param	_RowShift: Bit shift value for row.
* @param	_NumCols: Number of cols in the hardware.
* @param	_NumRows: Number of rows in the hardware.
* @param	_ShimRowNum: Row number of the shimrow.
* @param	_MemTileRowStart: Starting row number of the mem tile.
* @param	_MemTileNumRows: Number of mem tile rows.
* @param	_AieTileRowStart: Starting row number of the mem tile.
* @param	_AieTileNumRows: Number of mem tile rows.
*
* @return	None.
*
* @note		The macro declares it XAie_Config as a stack variable.
*
*******************************************************************************/
#define XAie_SetupConfig(Config, _AieGen, _BaseAddr, _ColShift, _RowShift,\
		_NumCols, _NumRows, _ShimRowNum, _MemTileRowStart, _MemTileNumRows,\
		_AieTileRowStart, _AieTileNumRows) \
		XAie_Config Config = {\
			.AieGen = _AieGen,\
			.BaseAddr = _BaseAddr,\
			.ColShift = _ColShift,\
			.RowShift = _RowShift,\
			.NumRows = _NumRows,\
			.NumCols = _NumCols,\
			.ShimRowNum = _ShimRowNum,\
			.MemTileRowStart = _MemTileRowStart,\
			.MemTileNumRows = _MemTileNumRows,\
			.AieTileRowStart = _AieTileRowStart,\
			.AieTileNumRows = _AieTileNumRows,\
		}\

/*****************************************************************************/
/**
*
* Macro to declare device instance.
*
* @param	Inst: Name of the Device Instance variable.
* @param	ConfigPtr: Pointer to the XAie_Config structure containing the
*			   hardware details..
*
* @return	None.
*
* @note		The instance of a device must be always declared using this
*		macro. In future, the same macro will be expanded to allocate
*		more memory from the user application for resource management.
*
*******************************************************************************/
#define XAie_InstDeclare(Inst, ConfigPtr) XAie_DevInst Inst = { 0 }

#endif	/* end of protection macro */

/** @} */
