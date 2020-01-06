/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* @file xaie_ss.c
* @{
*
* This file contains routines for AIE stream switch
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   10/21/2019  Optimize stream switch data structures
* 1.2   Tejus	01/04/2020  Cleanup error messages
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_ss.h"

/************************** Function Definitions *****************************/
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
static AieRC _XAie_GetSlaveIdx(const XAie_StrmMod *StrmMod, StrmSwPortType Slave,
		u8 PortNum, u8 *SlaveIdx)
{
	u32 BaseAddr;
	u32 RegAddr;
	const XAie_StrmPort *PortPtr;

	/* Get Base Addr of the slave tile from Stream Switch Module */
	BaseAddr = StrmMod->SlvConfigBaseAddr;

	PortPtr = &StrmMod->SlvConfig[Slave];

	/* Return error if the Slave Port Type is not valid */
	if((PortPtr->NumPorts == 0) || (PortNum >= PortPtr->NumPorts)) {
		XAieLib_print("Error: Invalid Slave Port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	RegAddr = PortPtr->PortBaseAddr + StrmMod->PortOffset * PortNum;
	*SlaveIdx = (RegAddr - BaseAddr) / 4;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to get the register offset and value required to configure
* the selected slave port of the stream switch in the corresponding tile.
*
* @param	PortPtr - Pointer to the internal port data structure.
* @param	PortNum - Port Number.
* @param	Enable - Enable/Disable the slave port (1-Enable,0-Disable).
* @param	PktEnable - Enable/Disable the packet switching mode
*		(1-Enable,0-Disable).
* @param	RegVal - pointer to store the register value.
* @param	RegOff - pointer to store the regster offset.
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		Internal API.
*
*******************************************************************************/
static AieRC _XAie_StrmConfigSlv(const XAie_StrmMod *StrmMod,
		StrmSwPortType PortType, u8 PortNum, u8 Enable, u8 PktEnable,
		u32 *RegVal, u32 *RegOff)
{
	*RegVal = 0U;
	const XAie_StrmPort  *PortPtr;

	/* Get the slave port pointer from stream module */
	PortPtr = &StrmMod->SlvConfig[PortType];

	if((PortPtr->NumPorts == 0) || (PortNum >= PortPtr->NumPorts)) {
		XAieLib_print("Error: Invalid Slave Port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	*RegOff = PortPtr->PortBaseAddr + StrmMod->PortOffset * PortNum;

	if(Enable != XAIE_ENABLE)
		return XAIE_OK;

	/* Frame the 32-bit reg value */
	*RegVal = XAie_SetField(Enable, StrmMod->SlvEn.Lsb,
			StrmMod->SlvEn.Mask) |
		XAie_SetField(PktEnable,
				StrmMod->SlvPktEn.Lsb, StrmMod->SlvEn.Mask);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to get the register offset and value required to configure
* the selected master port of the stream switch in the corresponding tile.
*
* @param	PortPtr - Pointer to the internal port data structure.
* @param	PortNum - Port Number.
* @param	Enable - Enable/Disable the slave port (1-Enable,0-Disable).
* @param	PktEnable - Enable/Disable the packet switching mode
*		(1-Enable,0-Disable).
* @param	RegVal - pointer to store the register value.
* @param	RegOff - pointer to store the regster offset.
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		Internal API.
*
*******************************************************************************/
static AieRC _StrmConfigMstr(const XAie_StrmMod *StrmMod,
		StrmSwPortType PortType, u8 PortNum, u8 Enable, u8 PktEnable,
		u8 Config, u32 *RegVal, u32 *RegOff)
{

	u8 DropHdr;
	*RegVal = 0U;
	const XAie_StrmPort *PortPtr;

	PortPtr = &StrmMod->MstrConfig[PortType];

	if((PortPtr->NumPorts == 0) || (PortNum >= PortPtr->NumPorts)) {
		XAieLib_print("Error: Invalid Stream Port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	*RegOff = PortPtr->PortBaseAddr + StrmMod->PortOffset * PortNum;
	if(Enable != XAIE_ENABLE)
		return XAIE_OK;

	/* Extract the drop header field */
	DropHdr = XAie_GetField(Config, StrmMod->DrpHdr.Lsb,
			StrmMod->DrpHdr.Mask);

	/* Frame 32-bit reg value */
	*RegVal = XAie_SetField(Enable, StrmMod->MstrEn.Lsb,
			StrmMod->MstrEn.Mask) |
		XAie_SetField(PktEnable, StrmMod->MstrPktEn.Lsb,
				StrmMod->MstrPktEn.Mask) |
		XAie_SetField(DropHdr, StrmMod->DrpHdr.Lsb,
				StrmMod->DrpHdr.Mask) |
		XAie_SetField(Config, StrmMod->Config.Lsb,
				StrmMod->Config.Mask);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to connect the selected master port to the specified slave
* port of the stream switch switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	Slave - Slave port type.
* @param	SlvPortNum- Slave port number.
* @param	Master - Master port type.
* @param	MstrPortNum- Master port number.
* @param	SlvEnable - Enable/Disable the slave port (1-Enable,0-Disable).
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StreamSwitchConfigureCct(XAie_DevInst *DevInst, XAie_LocRange Range,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum, u8 Enable)
{
	AieRC RC;
	u64 MstrAddr;
	u64 SlvAddr;
	u32 MstrOff;
	u32 MstrVal;
	u32 SlvOff;
	u32 SlvVal;
	u8 SlaveIdx;
	u8 TileType;
	const XAie_StrmMod *StrmMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Invalid Device Range\n");
		return XAIE_INVALID_RANGE;
	}

	if((Slave >= SS_PORT_TYPE_MAX) || (Master >= SS_PORT_TYPE_MAX)) {
		XAieLib_print("Error: Invalid Stream Switch Ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Range has different Tile Types\n");
		return XAIE_INVALID_RANGE;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;

	RC = _XAie_GetSlaveIdx(StrmMod, Slave, SlvPortNum, &SlaveIdx);
	if(RC != XAIE_OK) {
		XAieLib_print("Error: Unable to compute Slave Index\n");
		return RC;
	}

	/* Compute the register value and register address for the master port*/
	RC = _StrmConfigMstr(StrmMod, Master, MstrPortNum, Enable, XAIE_DISABLE,
			SlaveIdx, &MstrVal, &MstrOff);
	if(RC != XAIE_OK) {
		XAieLib_print("Error: Master config error\n");
		return RC;
	}

	/* Compute the register value and register address for slave port */
	RC = _XAie_StrmConfigSlv(StrmMod, Slave, SlvPortNum, Enable,
			XAIE_DISABLE, &SlvVal, &SlvOff);
	if(RC != XAIE_OK) {
		XAieLib_print("Error: Slave config error\n");
		return RC;
	}

	for(u8 R = Range.Start.Row; R <= Range.End.Row; R += Range.Stride.Row) {
		for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

			/* Compute absolute address and write to register */
			MstrAddr = DevInst->BaseAddr +
				_XAie_GetTileAddr(DevInst, R ,C) + MstrOff;
			SlvAddr = DevInst->BaseAddr +
				_XAie_GetTileAddr(DevInst, R ,C) + SlvOff;

			XAieGbl_Write32(MstrAddr, MstrVal);
			XAieGbl_Write32(SlvAddr, SlvVal);
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to enable the connection between the selected master port
* to the specified slave port of the stream switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	Slave - Slave port type.
* @param	SlvPortNum- Slave port number.
* @param	Master - Master port type.
* @param	MstrPortNum- Master port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StrmConnCctEnableRange(XAie_DevInst *DevInst, XAie_LocRange Range,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{

	return XAie_StreamSwitchConfigureCct(DevInst, Range, Slave, SlvPortNum,
			Master, MstrPortNum, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API is used to disable the connection between the selected master port
* to the specified slave port of the stream switch switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	Slave - Slave port type.
* @param	SlvPortNum- Slave port number.
* @param	Master - Master port type.
* @param	MstrPortNum- Master port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StrmConnCctDisableRange(XAie_DevInst *DevInst, XAie_LocRange Range,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{

	return XAie_StreamSwitchConfigureCct(DevInst, Range, Slave, SlvPortNum,
			Master, MstrPortNum, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API is used to enable the connection between the selected master port
* to the specified slave port of the stream switch switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StrmConnCctEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_StrmConnCctEnableRange(DevInst, Range, Slave, SlvPortNum,
			Master, MstrPortNum);

}

/*****************************************************************************/
/**
*
* This API is used to disable the connection between the selected master port
* to the specified slave port of the stream switch in ciruit switch mode.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param	Slave: Slave port type.
* @param	SlvPortNum: Slave port number.
* @param	Master: Master port type.
* @param	MstrPortNum: Master port number.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_StrmConnCctDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		StrmSwPortType Slave, u8 SlvPortNum, StrmSwPortType Master,
		u8 MstrPortNum)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_StrmConnCctDisableRange(DevInst, Range, Slave, SlvPortNum,
			Master, MstrPortNum);

}
