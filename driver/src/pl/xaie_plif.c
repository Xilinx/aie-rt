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
* @file xaie_plif.c
* @{
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   10/28/2019  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_plif.h"
#include "xaiegbl_defs.h"
#include "xaie_helper.h"

/************************** Constant Definitions *****************************/
#define XAIE_PLIF_WIDTH_64SHIFT 6U
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API sets the bypass register for a range of AIE tile on the PL2ME
* interface.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0, 1, 2, 4, 5, 6)
* @param	Enable: XAIE_DISABLE for disable, XAIE_ENABLE for enable
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API only.
*
******************************************************************************/
static AieRC _XAie_PlIfBliBypassRange(XAie_DevInst *DevInst,
		XAie_LocRange Range, u8 PortNum, u8 Enable)
{
	u8 TileType;
	u64 RegAddr;
	u32 FldVal;
	u32 Mask;
	const XAie_PlIfMod *PlIfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Invalid Device Range\n");
		return XAIE_INVALID_RANGE;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
			(TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Range has different Tile Types\n");
		return XAIE_INVALID_RANGE;
	}

	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;

	/*
	 * Ports 3 and 7 BLI Bypass is enabled in the hardware by default.
	 * Check and return error if the portnum is invalid.
	 */
	if((PortNum > PlIfMod->MaxByPassPortNum) || (PortNum == 3U) ||
			(PortNum == 7U)) {
		XAieLib_print("Error: Invalid Port Number\n");
		return XAIE_ERR_STREAM_PORT;
	}

	/* Port number 4-6 are mapped to bits 3-5 */
	if(PortNum > 3U)
		PortNum--;

	Mask = PlIfMod->DownSzrByPass[PortNum].Mask;
	FldVal = XAie_SetField(Enable, PlIfMod->DownSzrByPass[PortNum].Lsb,
			Mask);

	/* Iteration over Row is not needed */
	for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

		/* Compute register address */
		RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, 0U, C) +
			PlIfMod->DownSzrByPassOff;
		XAieGbl_MaskWrite32(RegAddr, Mask, FldVal);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures the Downsizer Enable register for stream ports connecting
* PL2AIE.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Enable: XAIE_DISABLE for disable, XAIE_ENABLE for enable
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API only.
*
******************************************************************************/
static AieRC _XAie_PlIfDownSzrPortEnableReg(XAie_DevInst *DevInst,
		XAie_LocRange Range, u8 PortNum, u8 Enable)
{
	u8 TileType;
	u64 RegAddr;
	u32 FldVal;
	u32 Mask;
	const XAie_PlIfMod *PlIfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Invalid Device Range\n");
		return XAIE_INVALID_RANGE;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
			(TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Range has different Tile Types\n");
		return XAIE_INVALID_RANGE;
	}

	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;

	if((PortNum > PlIfMod->NumDownSzrPorts)) {
		XAieLib_print("Error: Invalid Port Number\n");
		return XAIE_ERR_STREAM_PORT;
	}

	/* Enable or Disable stream port in PL2ME downsizer enable register */
	Mask = PlIfMod->DownSzrEn[PortNum].Mask;
	FldVal = XAie_SetField(Enable, PlIfMod->DownSzrEn[PortNum].Lsb, Mask);

	/* Iteration over Row is not needed */
	for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

		/* Compute register address */
		RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, 0U, C) +
			PlIfMod->DownSzrEnOff;
		XAieGbl_MaskWrite32(RegAddr, Mask, FldVal);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures the stream width for AIE->PL interfaces. The upsizer
* register is configured with the PortNumber provided. This is an internal API
* only and can be used to Enable or Disable AIE->PL interface.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0-5)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
* @param	Enable: XAIE_ENABLE or XAIE_DISABLE
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		When the width is 128 bits, port number can be any one of the
*		valid port numbers. Ex: For 4_5 combo, port number can be 4 or
*		5. Internal API only.
*
******************************************************************************/
static AieRC _XAie_AieToPlIntfConfig(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum, XAie_PlIfWidth Width, u8 Enable)
{
	u8 TileType;
	u8 Idx;
	u32 FldVal;
	u32 FldMask;
	u32 RegOff;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Invalid Device Range\n");
		return XAIE_INVALID_RANGE;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
			(TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Range has different Tile Types\n");
		return XAIE_INVALID_RANGE;
	}

	/* Check Width for validity */
	if((Width != PLIF_WIDTH_32) && (Width != PLIF_WIDTH_64) &&
			(Width != PLIF_WIDTH_128)) {
		XAieLib_print("Error: Invalid Width\n");
		return XAIE_INVALID_PLIF_WIDTH;
	}

	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;

	/* Setup field mask and field value for aie to pl interface */
	if(PortNum >= PlIfMod->NumDownSzrPorts) {
		XAieLib_print("Error: Invalid stream port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	if(Width == PLIF_WIDTH_128) {
		/*
		 * Get the register field to configure. Valid port
		 * numbers are 0-5. Divide the port number by 2 to get
		 * the right index. Two 64 Bit ports are combined to
		 * get a 128 Bit port.
		 */
		Idx = PortNum / 2U;
		FldMask = PlIfMod->UpSzr128Bit[Idx].Mask;
		FldVal = XAie_SetField(Enable,
				PlIfMod->UpSzr128Bit[Idx].Lsb,
				FldMask);
	} else {
		FldMask = PlIfMod->UpSzr32_64Bit[PortNum].Mask;
		/*
		 * Field Value has to be set to 1 for 64 Bit interface
		 * and 0 for 32 Bit interface
		 */
		FldVal = XAie_SetField(Width >> XAIE_PLIF_WIDTH_64SHIFT,
				PlIfMod->UpSzr32_64Bit[PortNum].Lsb,
				FldMask);
	}

	RegOff = PlIfMod->UpSzrOff;

	/* Iteration over Row is not needed */
	for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

		RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, 0U, C) + RegOff;
		/* Mask write to the upsizer register */
		XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures the stream width for PL->AIE interfaces. The downsizer
* register is configured with the port number provided by the user.
* Once the downsizer register is configured, the API also enables the ports in
* the downsizer enable register. The api configures the interface for a range
* of AIE Tiles.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
* @param	Enable: XAIE_ENABLE or XAIE_DISABLE
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If this API is used to configure PLTOAIE interfaces, explicit
*		call to enable stream ports in downsizer enable register is not
*		required. When configuring for 128 bit width, the user has to
*		provide one valid port. The api enables the other port by
*		default. Internal API only.
*
******************************************************************************/
static AieRC _XAie_PlToAieIntfConfig(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum, XAie_PlIfWidth Width, u8 Enable)
{
	u8 TileType;
	u8 Idx;
	u32 FldVal;
	u32 FldMask;
	u32 DwnSzrEnMask;
	u32 DwnSzrEnVal;
	u32 RegOff;
	u64 RegAddr;
	u64 DwnSzrEnRegAddr;
	const XAie_PlIfMod *PlIfMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Invalid Device Range\n");
		return XAIE_INVALID_RANGE;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
			(TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Range has different Tile Types\n");
		return XAIE_INVALID_RANGE;
	}

	/* Check Width for validity */
	if((Width != PLIF_WIDTH_32) && (Width != PLIF_WIDTH_64) &&
			(Width != PLIF_WIDTH_128)) {
		XAieLib_print("Error: Invalid Width\n");
		return XAIE_INVALID_PLIF_WIDTH;
	}

	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;

	/* Setup field mask and field value for pl to aie interface */
	if(PortNum >= PlIfMod->NumDownSzrPorts) {
		XAieLib_print("Error: Invalid stream port\n");
		return XAIE_ERR_STREAM_PORT;
	}

	if(Width == PLIF_WIDTH_128) {
		/*
		 * Get the register field to configure. Valid port
		 * numbers are 0-5. Divide the port number by 2 to get
		 * the right index. Two 64 Bit ports are combined to
		 * get a 128 Bit port.
		 */
		Idx = PortNum / 2U;
		FldMask = PlIfMod->DownSzr128Bit[Idx].Mask;
		FldVal = XAie_SetField(Enable,
				PlIfMod->DownSzr128Bit[Idx].Lsb,
				FldMask);
	} else {
		FldMask = PlIfMod->DownSzr32_64Bit[PortNum].Mask;
		/*
		 * Field Value has to be set to 1 for 64 Bit interface
		 * and 0 for 32 Bit interface. Width is shifted to move 64(2^6)
		 * to LSB. When width is 32, the shift results in 0.
		 */
		FldVal = XAie_SetField(Width >> XAIE_PLIF_WIDTH_64SHIFT,
				PlIfMod->DownSzr32_64Bit[PortNum].Lsb,
				FldMask);
	}

	/*
	 * For PL to AIE interfaces, once the downsizer register is configured,
	 * bits corresponding to the stream ports have to enabled in the
	 * downsizer enable register.
	 */
	DwnSzrEnMask = PlIfMod->DownSzrEn[PortNum].Mask;
	DwnSzrEnVal = XAie_SetField(Enable,
			PlIfMod->DownSzrEn[PortNum].Lsb, DwnSzrEnMask);

	/* If width is 128 bits, enable both ports */
	if(Width == PLIF_WIDTH_128) {
		PortNum = (PortNum % 2U) ? (PortNum - 1U) : (PortNum + 1U);

		DwnSzrEnMask |= PlIfMod->DownSzrEn[PortNum].Mask;
		DwnSzrEnVal |= XAie_SetField(Enable,
				PlIfMod->DownSzrEn[PortNum].Lsb,
				PlIfMod->DownSzrEn[PortNum].Mask);
	}

	RegOff = PlIfMod->DownSzrOff;

	/* Iteration over Row is not needed */
	for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

		RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, 0U, C) + RegOff;
		/* Mask write to the downsizer register */
		XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);
		/* Mast write to downsizer enable register */
		DwnSzrEnRegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, 0U, C) +
			PlIfMod->DownSzrEnOff;
		XAieGbl_MaskWrite32(DwnSzrEnRegAddr, DwnSzrEnMask,
				DwnSzrEnVal);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API Enables the stream port with width for PL->AIE interfaces. The
* downsizer register is configured with the port number provided by the user.
* Once the downsizer register is configured, the API also enables the ports in
* the downsizer enable register. The api configures the interface for a range
* of AIE Tiles.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If this API is used to configure PLTOAIE interfaces, explicit
*		call to enable stream ports in downsizer enable register is not
*		required.
*
******************************************************************************/
AieRC XAie_PlToAieIntfRangeEnable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum, XAie_PlIfWidth Width)
{
	return _XAie_PlToAieIntfConfig(DevInst, Range, PortNum, Width,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables the stream port with width for PL->AIE interfaces. The
* downsizer register is configured with the port number provided by the user.
* Once the downsizer register is configured, the API also enables the ports in
* the downsizer enable register. The api configures the interface for a range
* of AIE Tiles.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If this API is used to configure PLTOAIE interfaces, explicit
*		call to enable stream ports in downsizer enable register is not
*		required.
*
******************************************************************************/
AieRC XAie_PlToAieIntfRangeDisable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum, XAie_PlIfWidth Width)
{
	return _XAie_PlToAieIntfConfig(DevInst, Range, PortNum, Width,
			XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API Enables the stream port with width for PL->AIE interfaces. The
* downsizer register is configured with the port number provided by the user.
* Once the downsizer register is configured, the API also enables the ports in
* the downsizer enable register.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If this API is used to configure PLTOAIE interfaces, explicit
*		call to enable stream ports in downsizer enable register is not
*		required.
*
******************************************************************************/
AieRC XAie_PlToAieIntfEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_PlToAieIntfRangeEnable(DevInst, Range, PortNum, Width);
}

/*****************************************************************************/
/**
*
* This API Disables the stream port with width for PL->AIE interfaces. The
* downsizer register is configured with the port number provided by the user.
* Once the downsizer register is configured, the API also enables the ports in
* the downsizer enable register.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If this API is used to configure PLTOAIE interfaces, explicit
*		call to disable stream ports in downsizer enable register is not
*		required.
*
******************************************************************************/
AieRC XAie_PlToAieIntfDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_PlToAieIntfRangeDisable(DevInst, Range, PortNum, Width);
}

/*****************************************************************************/
/**
*
* This API Enables the stream width for AIE->PL interfaces. The upsizer
* register is configured with the PortNumber provided.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0-5)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_AieToPlIntfRangeEnable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum, XAie_PlIfWidth Width)
{
	return _XAie_AieToPlIntfConfig(DevInst, Range, PortNum, Width,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables the stream width for AIE->PL interfaces. The upsizer
* register is configured with the PortNumber provided.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0-5)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_AieToPlIntfRangeDisable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum, XAie_PlIfWidth Width)
{
	return _XAie_AieToPlIntfConfig(DevInst, Range, PortNum, Width,
			XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API Enables the stream width for AIE->PL interfaces. The upsizer
* register is configured with the PortNumber provided.
*
* @param	DevInst: Device Instance
* @param	Loc: Coodinate of AIE Tile
* @param        PortNum: Stream Port Number (0-5)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_AieToPlIntfEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_AieToPlIntfRangeEnable(DevInst, Range, PortNum, Width);
}

/*****************************************************************************/
/**
*
* This API Disables the stream width for AIE->PL interfaces. The upsizer
* register is configured with the PortNumber provided.
*
* @param	DevInst: Device Instance
* @param	Loc: Coodinate of AIE Tile
* @param        PortNum: Stream Port Number (0-5)
* @param	Width: Supported widths are 32, 64 and 128
*		(PLIF_WIDTH_32/64/128)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_AieToPlIntfDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_AieToPlIntfRangeDisable(DevInst, Range, PortNum, Width);
}

/*****************************************************************************/
/**
*
* This API Enables a stream port in the Downsizer Enable register for connecting
* PL2AIE. This enables for a range of tiles.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Enable: 0 for disable, 1 for enable
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfDownSzrRangeEnable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum)
{
	return _XAie_PlIfDownSzrPortEnableReg(DevInst, Range, PortNum,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables a stream port in the Downsizer Enable register for
* connecting PL2AIE. This is for a single tiles.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Enable: 0 for disable, 1 for enable
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfDownSzrRangeDisable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum)
{
	return _XAie_PlIfDownSzrPortEnableReg(DevInst, Range, PortNum,
			XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API Enables a stream port in the Downsizer Enable register for connecting
* PL2AIE. This is for a single tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Enable: 0 for disable, 1 for enable
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfDownSzrEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_PlIfDownSzrRangeEnable(DevInst, Range, PortNum);
}

/*****************************************************************************/
/**
*
* This API Disables a stream port in the Downsizer Enable register for
* connecting PL2AIE. This is for a single tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0-7)
* @param	Enable: 0 for disable, 1 for enable
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfDownSzrDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_PlIfDownSzrRangeDisable(DevInst, Range, PortNum);
}

/*****************************************************************************/
/**
*
* This API Enables the BLI bypass for a port number at the PL2ME interface, for
* a range of AIE tiles.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0, 1, 2, 4, 5, 6)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfBliBypassRangeEnable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum)
{
	return _XAie_PlIfBliBypassRange(DevInst, Range, PortNum,
			XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables the BLI bypass for a port number at the PL2ME interface,
* for a range of AIE tiles.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param        PortNum: Stream Port Number (0, 1, 2, 4, 5, 6)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfBliBypassRangeDisable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum)
{
	return _XAie_PlIfBliBypassRange(DevInst, Range, PortNum,
			XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API Enables the BLI bypass for a port number at the PL2ME interface, for
* a single AIE tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0, 1, 2, 4, 5, 6)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfBliBypassEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_PlIfBliBypassRangeEnable(DevInst, Range, PortNum);
}

/*****************************************************************************/
/**
*
* This API Disables the BLI bypass for a port number at the PL2ME interface, for
* a single AIE tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Loc of AIE Tiles
* @param        PortNum: Stream Port Number (0, 1, 2, 4, 5, 6)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_PlIfBliBypassDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_PlIfBliBypassRangeDisable(DevInst, Range, PortNum);
}
